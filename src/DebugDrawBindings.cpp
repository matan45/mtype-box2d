/*
 * Debug-draw integration. Each WorldState owns a b2DebugDraw whose function
 * pointers push vertex/color data into per-world scratch vectors. mType
 * calls world.draw() which clears the scratch, invokes b2World_Draw, and
 * leaves the scratch for the script to read out via debug_get_*_count /
 * debug_get_*(idx) accessors.
 *
 * The plugin has no SFML dependency: the readout natives return primitive
 * float arrays which the mType side can forward to any renderer.
 */

#include "PluginGlobals.hpp"
#include "BindingHelpers.hpp"
#include "WorldState.hpp"

#include "box2d/box2d.h"
#include "box2d/math_functions.h"
#include "box2d/types.h"

namespace mtypeb2d
{
    namespace
    {
        constexpr const char* kEx = "Box2DError";
        using namespace detail;

        /* ---- b2DebugDraw callbacks. context = WorldState*. ---- */

        void onDrawPolygon(const b2Vec2* verts, int n, b2HexColor color, void* ctx)
        {
            auto* ws = static_cast<WorldState*>(ctx);
            DebugPolygon p;
            p.verts.reserve(2 * n);
            for (int i = 0; i < n; ++i) {
                p.verts.push_back(verts[i].x);
                p.verts.push_back(verts[i].y);
            }
            p.color = static_cast<uint32_t>(color);
            ws->polygons.push_back(std::move(p));
        }

        void onDrawSolidPolygon(b2Transform xf, const b2Vec2* verts, int n,
                                float /*radius*/, b2HexColor color, void* ctx)
        {
            auto* ws = static_cast<WorldState*>(ctx);
            DebugPolygon p;
            p.verts.reserve(2 * n);
            for (int i = 0; i < n; ++i) {
                b2Vec2 w = b2TransformPoint(xf, verts[i]);
                p.verts.push_back(w.x);
                p.verts.push_back(w.y);
            }
            p.color = static_cast<uint32_t>(color);
            ws->polygons.push_back(std::move(p));
        }

        void onDrawCircle(b2Vec2 center, float radius, b2HexColor color, void* ctx)
        {
            static_cast<WorldState*>(ctx)->circles.push_back(
                DebugCircle{center.x, center.y, radius, static_cast<uint32_t>(color)});
        }

        void onDrawSolidCircle(b2Transform xf, float radius, b2HexColor color, void* ctx)
        {
            static_cast<WorldState*>(ctx)->circles.push_back(
                DebugCircle{xf.p.x, xf.p.y, radius, static_cast<uint32_t>(color)});
        }

        void onDrawSolidCapsule(b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color, void* ctx)
        {
            /* Approximate as two circles + a connecting segment. */
            auto* ws = static_cast<WorldState*>(ctx);
            ws->circles.push_back(DebugCircle{p1.x, p1.y, radius, static_cast<uint32_t>(color)});
            ws->circles.push_back(DebugCircle{p2.x, p2.y, radius, static_cast<uint32_t>(color)});
            ws->segments.push_back(DebugSegment{p1.x, p1.y, p2.x, p2.y, static_cast<uint32_t>(color)});
        }

        void onDrawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* ctx)
        {
            static_cast<WorldState*>(ctx)->segments.push_back(
                DebugSegment{p1.x, p1.y, p2.x, p2.y, static_cast<uint32_t>(color)});
        }

        void onDrawTransform(b2Transform xf, void* ctx)
        {
            static_cast<WorldState*>(ctx)->transforms.push_back(
                DebugTransform{xf.p.x, xf.p.y, xf.q.c, xf.q.s});
        }

        void onDrawPoint(b2Vec2 p, float size, b2HexColor color, void* ctx)
        {
            /* No dedicated point list — render as a tiny circle. */
            static_cast<WorldState*>(ctx)->circles.push_back(
                DebugCircle{p.x, p.y, size * 0.5f, static_cast<uint32_t>(color)});
        }

        /* No-op DrawString — mType has its own text rendering via SFML. */
        void onDrawString(b2Vec2, const char*, b2HexColor, void*) {}

        /* Apply the WorldState-owned debug-draw config to its b2DebugDraw
         * struct. Called once at world creation in PluginEntry.cpp's
         * createWorldState. Re-applied each time the script changes flags
         * via debug_draw_set_flags. */
        void wireDebugDraw(WorldState& ws)
        {
            ws.debugDraw = b2DefaultDebugDraw();
            ws.debugDraw.DrawPolygonFcn      = &onDrawPolygon;
            ws.debugDraw.DrawSolidPolygonFcn = &onDrawSolidPolygon;
            ws.debugDraw.DrawCircleFcn       = &onDrawCircle;
            ws.debugDraw.DrawSolidCircleFcn  = &onDrawSolidCircle;
            ws.debugDraw.DrawSolidCapsuleFcn = &onDrawSolidCapsule;
            ws.debugDraw.DrawSegmentFcn      = &onDrawSegment;
            ws.debugDraw.DrawTransformFcn    = &onDrawTransform;
            ws.debugDraw.DrawPointFcn        = &onDrawPoint;
            ws.debugDraw.DrawStringFcn       = &onDrawString;
            ws.debugDraw.context             = &ws;
        }

        /* ---- world_draw: clear scratch, invoke b2World_Draw ---- */

        MTypeValue* nWorldDraw(void*, MTypeContext* ctx,
                               const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_world_draw", kEx))
                return g_host->makeVoid(ctx);
            int64_t key = g_host->getInt(args[0]);
            b2WorldId id = intToWorld(key);
            if (!validateWorld(id, ctx, "__native__b2d_world_draw", kEx))
                return g_host->makeVoid(ctx);
            WorldState* ws = findWorldState(key);
            if (!ws) return g_host->makeVoid(ctx);
            ws->polygons.clear();
            ws->circles.clear();
            ws->segments.clear();
            ws->transforms.clear();
            /* Re-wire in case callbacks haven't been initialized yet
             * (defensive — createWorldState already calls wireDebugDraw). */
            if (ws->debugDraw.DrawPolygonFcn == nullptr) wireDebugDraw(*ws);
            b2World_Draw(id, &ws->debugDraw);
            return g_host->makeVoid(ctx);
        }

        /* ---- flag setters ---- */

        MTypeValue* nDebugDrawSetFlags(void*, MTypeContext* ctx,
                                       const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 7, "__native__b2d_debug_draw_set_flags", kEx))
                return g_host->makeVoid(ctx);
            int64_t key = g_host->getInt(args[0]);
            WorldState* ws = findWorldState(key);
            if (!ws) {
                g_host->raiseError(ctx, kEx, "debug_draw_set_flags: unknown world id");
                return g_host->makeVoid(ctx);
            }
            ws->debugDraw.drawShapes  = getB(args[1]);
            ws->debugDraw.drawJoints  = getB(args[2]);
            ws->debugDraw.drawBounds  = getB(args[3]);
            ws->debugDraw.drawMass    = getB(args[4]);
            ws->debugDraw.drawContacts = getB(args[5]);
            ws->debugDraw.drawContactNormals = getB(args[6]);
            return g_host->makeVoid(ctx);
        }

        /* ---- readout natives ---- */

        MTypeValue* nDebugPolygonCount(void*, MTypeContext* ctx,
                                       const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_debug_get_polygon_count", kEx))
                return g_host->makeInt(ctx, 0);
            WorldState* ws = findWorldState(g_host->getInt(args[0]));
            return g_host->makeInt(ctx, ws ? static_cast<int64_t>(ws->polygons.size()) : 0);
        }
        MTypeValue* nDebugPolygonAt(void*, MTypeContext* ctx,
                                    const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 2, "__native__b2d_debug_get_polygon", kEx))
                return emptyFloatArray(ctx);
            WorldState* ws = findWorldState(g_host->getInt(args[0]));
            if (!ws) return emptyFloatArray(ctx);
            int idx = getI(args[1]);
            if (idx < 0 || idx >= static_cast<int>(ws->polygons.size())) return emptyFloatArray(ctx);
            const auto& p = ws->polygons[idx];
            /* Layout: [vertCount, color, x0, y0, x1, y1, ...] */
            int n = static_cast<int>(p.verts.size() / 2);
            MTypeValue* out = g_host->makeArray(ctx, MT_TAG_FLOAT, 2 + 2 * n);
            g_host->arraySet(out, 0, g_host->makeFloat(ctx, static_cast<double>(n)));
            g_host->arraySet(out, 1, g_host->makeFloat(ctx, static_cast<double>(p.color)));
            for (int i = 0; i < 2 * n; ++i)
                g_host->arraySet(out, 2 + i, g_host->makeFloat(ctx, p.verts[i]));
            return out;
        }

        MTypeValue* nDebugCircleCount(void*, MTypeContext* ctx,
                                      const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_debug_get_circle_count", kEx))
                return g_host->makeInt(ctx, 0);
            WorldState* ws = findWorldState(g_host->getInt(args[0]));
            return g_host->makeInt(ctx, ws ? static_cast<int64_t>(ws->circles.size()) : 0);
        }
        MTypeValue* nDebugCircleAt(void*, MTypeContext* ctx,
                                   const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 2, "__native__b2d_debug_get_circle", kEx))
                return emptyFloatArray(ctx);
            WorldState* ws = findWorldState(g_host->getInt(args[0]));
            if (!ws) return emptyFloatArray(ctx);
            int idx = getI(args[1]);
            if (idx < 0 || idx >= static_cast<int>(ws->circles.size())) return emptyFloatArray(ctx);
            const auto& c = ws->circles[idx];
            /* Layout: [cx, cy, r, color] */
            return makeFloatQuad(ctx, c.cx, c.cy, c.radius, static_cast<double>(c.color));
        }

        MTypeValue* nDebugSegmentCount(void*, MTypeContext* ctx,
                                       const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_debug_get_segment_count", kEx))
                return g_host->makeInt(ctx, 0);
            WorldState* ws = findWorldState(g_host->getInt(args[0]));
            return g_host->makeInt(ctx, ws ? static_cast<int64_t>(ws->segments.size()) : 0);
        }
        MTypeValue* nDebugSegmentAt(void*, MTypeContext* ctx,
                                    const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 2, "__native__b2d_debug_get_segment", kEx))
                return emptyFloatArray(ctx);
            WorldState* ws = findWorldState(g_host->getInt(args[0]));
            if (!ws) return emptyFloatArray(ctx);
            int idx = getI(args[1]);
            if (idx < 0 || idx >= static_cast<int>(ws->segments.size())) return emptyFloatArray(ctx);
            const auto& s = ws->segments[idx];
            /* Layout: [x1, y1, x2, y2, color] */
            MTypeValue* out = g_host->makeArray(ctx, MT_TAG_FLOAT, 5);
            g_host->arraySet(out, 0, g_host->makeFloat(ctx, s.x1));
            g_host->arraySet(out, 1, g_host->makeFloat(ctx, s.y1));
            g_host->arraySet(out, 2, g_host->makeFloat(ctx, s.x2));
            g_host->arraySet(out, 3, g_host->makeFloat(ctx, s.y2));
            g_host->arraySet(out, 4, g_host->makeFloat(ctx, static_cast<double>(s.color)));
            return out;
        }

        MTypeValue* nDebugTransformCount(void*, MTypeContext* ctx,
                                         const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_debug_get_transform_count", kEx))
                return g_host->makeInt(ctx, 0);
            WorldState* ws = findWorldState(g_host->getInt(args[0]));
            return g_host->makeInt(ctx, ws ? static_cast<int64_t>(ws->transforms.size()) : 0);
        }
        MTypeValue* nDebugTransformAt(void*, MTypeContext* ctx,
                                      const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 2, "__native__b2d_debug_get_transform", kEx))
                return emptyFloatArray(ctx);
            WorldState* ws = findWorldState(g_host->getInt(args[0]));
            if (!ws) return emptyFloatArray(ctx);
            int idx = getI(args[1]);
            if (idx < 0 || idx >= static_cast<int>(ws->transforms.size())) return emptyFloatArray(ctx);
            const auto& t = ws->transforms[idx];
            return makeFloatQuad(ctx, t.px, t.py, t.cosA, t.sinA);
        }
    }

    /* Called from PluginEntry.cpp when a WorldState is first created so the
     * b2DebugDraw struct is ready to use the moment a script invokes
     * world.draw(). */
    void initWorldDebugDraw(WorldState& ws)
    {
        wireDebugDraw(ws);
    }

    void registerDebugDrawNatives(MTypeContext* ctx)
    {
        Registrar r{ctx, "__native__b2d_"};
        r("world_draw",                    &nWorldDraw)
         ("debug_draw_set_flags",          &nDebugDrawSetFlags)
         ("debug_get_polygon_count",       &nDebugPolygonCount)
         ("debug_get_polygon",             &nDebugPolygonAt)
         ("debug_get_circle_count",        &nDebugCircleCount)
         ("debug_get_circle",              &nDebugCircleAt)
         ("debug_get_segment_count",       &nDebugSegmentCount)
         ("debug_get_segment",             &nDebugSegmentAt)
         ("debug_get_transform_count",     &nDebugTransformCount)
         ("debug_get_transform",           &nDebugTransformAt);
    }
}
