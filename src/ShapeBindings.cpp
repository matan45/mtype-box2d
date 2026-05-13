/*
 * Shape creation (circle / box / polygon / capsule / segment / chain) plus
 * per-shape property setters/getters and the v3 per-shape event opt-ins.
 *
 * v3 NOTE: contact events, hit events, and sensor events are OFF by default
 * for every shape. Scripts must call shape.enableContactEvents(true) etc.
 * to start seeing those event types in EventBindings drains.
 */

#include "PluginGlobals.hpp"
#include "BindingHelpers.hpp"

#include "box2d/box2d.h"
#include "box2d/collision.h"
#include "box2d/math_functions.h"

#include <vector>

namespace mtypeb2d
{
    namespace
    {
        constexpr const char* kEx = "Box2DError";
        using namespace detail;

        /* ---- shape factories ---- */

        MTypeValue* nShapeCreateCircle(void*, MTypeContext* ctx,
                                       const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 5, "__native__b2d_shape_create_circle", kEx))
                return g_host->makeInt(ctx, 0);
            b2BodyId body = intToBody(g_host->getInt(args[0]));
            if (!validateBody(body, ctx, "__native__b2d_shape_create_circle", kEx))
                return g_host->makeInt(ctx, 0);
            auto* def = findDefOrRaise(g_shapeDefs, g_host->getInt(args[1]), ctx,
                                       "__native__b2d_shape_create_circle", kEx);
            if (!def) return g_host->makeInt(ctx, 0);
            b2Circle c;
            c.center.x = getF(args[2]);
            c.center.y = getF(args[3]);
            c.radius   = getF(args[4]);
            b2ShapeId id = b2CreateCircleShape(body, def, &c);
            return g_host->makeInt(ctx, shapeToInt(id));
        }

        MTypeValue* nShapeCreateBox(void*, MTypeContext* ctx,
                                    const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 4, "__native__b2d_shape_create_box", kEx))
                return g_host->makeInt(ctx, 0);
            b2BodyId body = intToBody(g_host->getInt(args[0]));
            if (!validateBody(body, ctx, "__native__b2d_shape_create_box", kEx))
                return g_host->makeInt(ctx, 0);
            auto* def = findDefOrRaise(g_shapeDefs, g_host->getInt(args[1]), ctx,
                                       "__native__b2d_shape_create_box", kEx);
            if (!def) return g_host->makeInt(ctx, 0);
            b2Polygon poly = b2MakeBox(getF(args[2]), getF(args[3]));
            b2ShapeId id = b2CreatePolygonShape(body, def, &poly);
            return g_host->makeInt(ctx, shapeToInt(id));
        }

        MTypeValue* nShapeCreatePolygon(void*, MTypeContext* ctx,
                                        const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 3, "__native__b2d_shape_create_polygon", kEx))
                return g_host->makeInt(ctx, 0);
            b2BodyId body = intToBody(g_host->getInt(args[0]));
            if (!validateBody(body, ctx, "__native__b2d_shape_create_polygon", kEx))
                return g_host->makeInt(ctx, 0);
            auto* def = findDefOrRaise(g_shapeDefs, g_host->getInt(args[1]), ctx,
                                       "__native__b2d_shape_create_polygon", kEx);
            if (!def) return g_host->makeInt(ctx, 0);
            const MTypeValue* arr = args[2];
            if (g_host->getTag(arr) != MT_TAG_ARRAY) {
                g_host->raiseError(ctx, kEx, "shape_create_polygon: vertices must be a float[]");
                return g_host->makeInt(ctx, 0);
            }
            size_t flatLen = g_host->arrayLen(arr);
            if (flatLen < 6 || (flatLen % 2) != 0) {
                g_host->raiseError(ctx, kEx, "shape_create_polygon: need at least 3 points, even-length flat float[]");
                return g_host->makeInt(ctx, 0);
            }
            int n = static_cast<int>(flatLen / 2);
            if (n > B2_MAX_POLYGON_VERTICES) n = B2_MAX_POLYGON_VERTICES;
            std::vector<b2Vec2> points(n);
            for (int i = 0; i < n; ++i) {
                points[i].x = static_cast<float>(g_host->getFloat(g_host->arrayGet(ctx, arr, 2 * i)));
                points[i].y = static_cast<float>(g_host->getFloat(g_host->arrayGet(ctx, arr, 2 * i + 1)));
            }
            b2Hull hull = b2ComputeHull(points.data(), n);
            if (hull.count < 3) {
                g_host->raiseError(ctx, kEx, "shape_create_polygon: hull computation failed (degenerate points?)");
                return g_host->makeInt(ctx, 0);
            }
            b2Polygon poly = b2MakePolygon(&hull, 0.0f);
            b2ShapeId id = b2CreatePolygonShape(body, def, &poly);
            return g_host->makeInt(ctx, shapeToInt(id));
        }

        MTypeValue* nShapeCreateCapsule(void*, MTypeContext* ctx,
                                        const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 7, "__native__b2d_shape_create_capsule", kEx))
                return g_host->makeInt(ctx, 0);
            b2BodyId body = intToBody(g_host->getInt(args[0]));
            if (!validateBody(body, ctx, "__native__b2d_shape_create_capsule", kEx))
                return g_host->makeInt(ctx, 0);
            auto* def = findDefOrRaise(g_shapeDefs, g_host->getInt(args[1]), ctx,
                                       "__native__b2d_shape_create_capsule", kEx);
            if (!def) return g_host->makeInt(ctx, 0);
            b2Capsule cap;
            cap.center1.x = getF(args[2]);
            cap.center1.y = getF(args[3]);
            cap.center2.x = getF(args[4]);
            cap.center2.y = getF(args[5]);
            cap.radius    = getF(args[6]);
            b2ShapeId id = b2CreateCapsuleShape(body, def, &cap);
            return g_host->makeInt(ctx, shapeToInt(id));
        }

        MTypeValue* nShapeCreateSegment(void*, MTypeContext* ctx,
                                        const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 6, "__native__b2d_shape_create_segment", kEx))
                return g_host->makeInt(ctx, 0);
            b2BodyId body = intToBody(g_host->getInt(args[0]));
            if (!validateBody(body, ctx, "__native__b2d_shape_create_segment", kEx))
                return g_host->makeInt(ctx, 0);
            auto* def = findDefOrRaise(g_shapeDefs, g_host->getInt(args[1]), ctx,
                                       "__native__b2d_shape_create_segment", kEx);
            if (!def) return g_host->makeInt(ctx, 0);
            b2Segment seg;
            seg.point1.x = getF(args[2]);
            seg.point1.y = getF(args[3]);
            seg.point2.x = getF(args[4]);
            seg.point2.y = getF(args[5]);
            b2ShapeId id = b2CreateSegmentShape(body, def, &seg);
            return g_host->makeInt(ctx, shapeToInt(id));
        }

        /* ---- chains ---- */

        MTypeValue* nChainCreate(void*, MTypeContext* ctx,
                                 const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 5, "__native__b2d_chain_create", kEx))
                return g_host->makeInt(ctx, 0);
            b2BodyId body = intToBody(g_host->getInt(args[0]));
            if (!validateBody(body, ctx, "__native__b2d_chain_create", kEx))
                return g_host->makeInt(ctx, 0);

            const MTypeValue* arr = args[1];
            if (g_host->getTag(arr) != MT_TAG_ARRAY) {
                g_host->raiseError(ctx, kEx, "chain_create: points must be a float[]");
                return g_host->makeInt(ctx, 0);
            }
            size_t flatLen = g_host->arrayLen(arr);
            if (flatLen < 8 || (flatLen % 2) != 0) {
                g_host->raiseError(ctx, kEx, "chain_create: need at least 4 points, even-length flat float[]");
                return g_host->makeInt(ctx, 0);
            }
            int n = static_cast<int>(flatLen / 2);
            std::vector<b2Vec2> points(n);
            for (int i = 0; i < n; ++i) {
                points[i].x = static_cast<float>(g_host->getFloat(g_host->arrayGet(ctx, arr, 2 * i)));
                points[i].y = static_cast<float>(g_host->getFloat(g_host->arrayGet(ctx, arr, 2 * i + 1)));
            }
            bool isLoop = getB(args[2]);
            float friction    = getF(args[3]);
            float restitution = getF(args[4]);

            b2ChainDef def = b2DefaultChainDef();
            def.points = points.data();
            def.count  = n;
            def.isLoop = isLoop;
            b2SurfaceMaterial mat = b2DefaultSurfaceMaterial();
            mat.friction    = friction;
            mat.restitution = restitution;
            def.materials = &mat;
            def.materialCount = 1;

            b2ChainId id = b2CreateChain(body, &def);
            return g_host->makeInt(ctx, chainToInt(id));
        }

        MTypeValue* nChainDestroy(void*, MTypeContext* ctx,
                                  const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_chain_destroy", kEx))
                return g_host->makeVoid(ctx);
            b2ChainId id = intToChain(g_host->getInt(args[0]));
            if (b2Chain_IsValid(id)) b2DestroyChain(id);
            return g_host->makeVoid(ctx);
        }

        MTypeValue* nChainGetSegments(void*, MTypeContext* ctx,
                                      const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_chain_get_segments", kEx))
                return emptyIntArray(ctx);
            b2ChainId id = intToChain(g_host->getInt(args[0]));
            if (!validateChain(id, ctx, "__native__b2d_chain_get_segments", kEx))
                return emptyIntArray(ctx);
            int n = b2Chain_GetSegmentCount(id);
            std::vector<b2ShapeId> segs(n);
            if (n > 0) b2Chain_GetSegments(id, segs.data(), n);
            MTypeValue* out = g_host->makeArray(ctx, MT_TAG_INT, n);
            for (int i = 0; i < n; ++i)
                g_host->arraySet(out, i, g_host->makeInt(ctx, shapeToInt(segs[i])));
            return out;
        }

        /* ---- destroy / type / body lookup ---- */

        MTypeValue* nShapeDestroy(void*, MTypeContext* ctx,
                                  const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 2, "__native__b2d_shape_destroy", kEx))
                return g_host->makeVoid(ctx);
            b2ShapeId id = intToShape(g_host->getInt(args[0]));
            if (!b2Shape_IsValid(id)) return g_host->makeVoid(ctx);
            b2DestroyShape(id, getB(args[1]));
            return g_host->makeVoid(ctx);
        }

        MTypeValue* nShapeGetType(void*, MTypeContext* ctx,
                                  const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_shape_get_type", kEx))
                return g_host->makeInt(ctx, 0);
            b2ShapeId id = intToShape(g_host->getInt(args[0]));
            if (!validateShape(id, ctx, "__native__b2d_shape_get_type", kEx))
                return g_host->makeInt(ctx, 0);
            return g_host->makeInt(ctx, static_cast<int>(b2Shape_GetType(id)));
        }

        MTypeValue* nShapeGetBody(void*, MTypeContext* ctx,
                                  const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_shape_get_body", kEx))
                return g_host->makeInt(ctx, 0);
            b2ShapeId id = intToShape(g_host->getInt(args[0]));
            if (!validateShape(id, ctx, "__native__b2d_shape_get_body", kEx))
                return g_host->makeInt(ctx, 0);
            return g_host->makeInt(ctx, bodyToInt(b2Shape_GetBody(id)));
        }

        /* ---- per-shape property setters/getters ---- */

        #define SHAPE_SET1F(NAME, FN) \
        MTypeValue* nShape##NAME(void*, MTypeContext* ctx, \
                                  const MTypeValue* const* args, int argc) \
        { \
            if (!requireArgs(ctx, argc, 2, "__native__b2d_shape_" #NAME, kEx)) \
                return g_host->makeVoid(ctx); \
            b2ShapeId id = intToShape(g_host->getInt(args[0])); \
            if (!validateShape(id, ctx, "__native__b2d_shape_" #NAME, kEx)) \
                return g_host->makeVoid(ctx); \
            FN(id, getF(args[1])); \
            return g_host->makeVoid(ctx); \
        }

        SHAPE_SET1F(set_friction,    b2Shape_SetFriction)
        SHAPE_SET1F(set_restitution, b2Shape_SetRestitution)

        #undef SHAPE_SET1F

        MTypeValue* nShapeSetDensity(void*, MTypeContext* ctx,
                                     const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 3, "__native__b2d_shape_set_density", kEx))
                return g_host->makeVoid(ctx);
            b2ShapeId id = intToShape(g_host->getInt(args[0]));
            if (!validateShape(id, ctx, "__native__b2d_shape_set_density", kEx))
                return g_host->makeVoid(ctx);
            b2Shape_SetDensity(id, getF(args[1]), getB(args[2]));
            return g_host->makeVoid(ctx);
        }

        #define SHAPE_GET1F(NAME, FN) \
        MTypeValue* nShape##NAME(void*, MTypeContext* ctx, \
                                  const MTypeValue* const* args, int argc) \
        { \
            if (!requireArgs(ctx, argc, 1, "__native__b2d_shape_" #NAME, kEx)) \
                return g_host->makeFloat(ctx, 0.0); \
            b2ShapeId id = intToShape(g_host->getInt(args[0])); \
            if (!validateShape(id, ctx, "__native__b2d_shape_" #NAME, kEx)) \
                return g_host->makeFloat(ctx, 0.0); \
            return g_host->makeFloat(ctx, FN(id)); \
        }

        SHAPE_GET1F(get_density,     b2Shape_GetDensity)
        SHAPE_GET1F(get_friction,    b2Shape_GetFriction)
        SHAPE_GET1F(get_restitution, b2Shape_GetRestitution)

        #undef SHAPE_GET1F

        MTypeValue* nShapeIsSensor(void*, MTypeContext* ctx,
                                   const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_shape_is_sensor", kEx))
                return g_host->makeBool(ctx, 0);
            b2ShapeId id = intToShape(g_host->getInt(args[0]));
            if (!validateShape(id, ctx, "__native__b2d_shape_is_sensor", kEx))
                return g_host->makeBool(ctx, 0);
            return g_host->makeBool(ctx, b2Shape_IsSensor(id) ? 1 : 0);
        }

        MTypeValue* nShapeSetFilter(void*, MTypeContext* ctx,
                                    const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 4, "__native__b2d_shape_set_filter", kEx))
                return g_host->makeVoid(ctx);
            b2ShapeId id = intToShape(g_host->getInt(args[0]));
            if (!validateShape(id, ctx, "__native__b2d_shape_set_filter", kEx))
                return g_host->makeVoid(ctx);
            b2Filter f{};
            f.categoryBits = getU64(args[1]);
            f.maskBits     = getU64(args[2]);
            f.groupIndex   = getI(args[3]);
            b2Shape_SetFilter(id, f);
            return g_host->makeVoid(ctx);
        }

        MTypeValue* nShapeGetFilter(void*, MTypeContext* ctx,
                                    const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_shape_get_filter", kEx))
                return emptyIntArray(ctx);
            b2ShapeId id = intToShape(g_host->getInt(args[0]));
            if (!validateShape(id, ctx, "__native__b2d_shape_get_filter", kEx))
                return emptyIntArray(ctx);
            b2Filter f = b2Shape_GetFilter(id);
            MTypeValue* out = g_host->makeArray(ctx, MT_TAG_INT, 3);
            g_host->arraySet(out, 0, g_host->makeInt(ctx, static_cast<int64_t>(f.categoryBits)));
            g_host->arraySet(out, 1, g_host->makeInt(ctx, static_cast<int64_t>(f.maskBits)));
            g_host->arraySet(out, 2, g_host->makeInt(ctx, f.groupIndex));
            return out;
        }

        #define SHAPE_ENABLE1B(NAME, FN) \
        MTypeValue* nShape##NAME(void*, MTypeContext* ctx, \
                                  const MTypeValue* const* args, int argc) \
        { \
            if (!requireArgs(ctx, argc, 2, "__native__b2d_shape_" #NAME, kEx)) \
                return g_host->makeVoid(ctx); \
            b2ShapeId id = intToShape(g_host->getInt(args[0])); \
            if (!validateShape(id, ctx, "__native__b2d_shape_" #NAME, kEx)) \
                return g_host->makeVoid(ctx); \
            FN(id, getB(args[1])); \
            return g_host->makeVoid(ctx); \
        }

        SHAPE_ENABLE1B(enable_sensor_events,  b2Shape_EnableSensorEvents)
        SHAPE_ENABLE1B(enable_contact_events, b2Shape_EnableContactEvents)
        SHAPE_ENABLE1B(enable_hit_events,     b2Shape_EnableHitEvents)
        SHAPE_ENABLE1B(enable_pre_solve_events, b2Shape_EnablePreSolveEvents)

        #undef SHAPE_ENABLE1B

        MTypeValue* nShapeGetAABB(void*, MTypeContext* ctx,
                                  const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_shape_get_aabb", kEx))
                return emptyFloatArray(ctx);
            b2ShapeId id = intToShape(g_host->getInt(args[0]));
            if (!validateShape(id, ctx, "__native__b2d_shape_get_aabb", kEx))
                return emptyFloatArray(ctx);
            b2AABB aabb = b2Shape_GetAABB(id);
            return makeFloatQuad(ctx, aabb.lowerBound.x, aabb.lowerBound.y,
                                       aabb.upperBound.x, aabb.upperBound.y);
        }

        /* ---- shape geometry readback (for debug-draw without b2World_Draw) ---- */

        MTypeValue* nShapeGetCircle(void*, MTypeContext* ctx,
                                    const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_shape_get_circle", kEx))
                return emptyFloatArray(ctx);
            b2ShapeId id = intToShape(g_host->getInt(args[0]));
            if (!validateShape(id, ctx, "__native__b2d_shape_get_circle", kEx))
                return emptyFloatArray(ctx);
            b2Circle c = b2Shape_GetCircle(id);
            return makeFloatTriple(ctx, c.center.x, c.center.y, c.radius);
        }

        MTypeValue* nShapeGetSegment(void*, MTypeContext* ctx,
                                     const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_shape_get_segment", kEx))
                return emptyFloatArray(ctx);
            b2ShapeId id = intToShape(g_host->getInt(args[0]));
            if (!validateShape(id, ctx, "__native__b2d_shape_get_segment", kEx))
                return emptyFloatArray(ctx);
            b2Segment s = b2Shape_GetSegment(id);
            return makeFloatQuad(ctx, s.point1.x, s.point1.y, s.point2.x, s.point2.y);
        }

        MTypeValue* nShapeGetPolygon(void*, MTypeContext* ctx,
                                     const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_shape_get_polygon", kEx))
                return emptyFloatArray(ctx);
            b2ShapeId id = intToShape(g_host->getInt(args[0]));
            if (!validateShape(id, ctx, "__native__b2d_shape_get_polygon", kEx))
                return emptyFloatArray(ctx);
            b2Polygon p = b2Shape_GetPolygon(id);
            MTypeValue* out = g_host->makeArray(ctx, MT_TAG_FLOAT, 2 * p.count);
            for (int i = 0; i < p.count; ++i) {
                g_host->arraySet(out, 2 * i,     g_host->makeFloat(ctx, p.vertices[i].x));
                g_host->arraySet(out, 2 * i + 1, g_host->makeFloat(ctx, p.vertices[i].y));
            }
            return out;
        }

        MTypeValue* nShapeGetCapsule(void*, MTypeContext* ctx,
                                     const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_shape_get_capsule", kEx))
                return emptyFloatArray(ctx);
            b2ShapeId id = intToShape(g_host->getInt(args[0]));
            if (!validateShape(id, ctx, "__native__b2d_shape_get_capsule", kEx))
                return emptyFloatArray(ctx);
            b2Capsule c = b2Shape_GetCapsule(id);
            MTypeValue* out = g_host->makeArray(ctx, MT_TAG_FLOAT, 5);
            g_host->arraySet(out, 0, g_host->makeFloat(ctx, c.center1.x));
            g_host->arraySet(out, 1, g_host->makeFloat(ctx, c.center1.y));
            g_host->arraySet(out, 2, g_host->makeFloat(ctx, c.center2.x));
            g_host->arraySet(out, 3, g_host->makeFloat(ctx, c.center2.y));
            g_host->arraySet(out, 4, g_host->makeFloat(ctx, c.radius));
            return out;
        }

        MTypeValue* nShapeTestPoint(void*, MTypeContext* ctx,
                                    const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 3, "__native__b2d_shape_test_point", kEx))
                return g_host->makeBool(ctx, 0);
            b2ShapeId id = intToShape(g_host->getInt(args[0]));
            if (!validateShape(id, ctx, "__native__b2d_shape_test_point", kEx))
                return g_host->makeBool(ctx, 0);
            return g_host->makeBool(ctx, b2Shape_TestPoint(id, getVec2(args[1], args[2])) ? 1 : 0);
        }
    }

    void registerShapeNatives(MTypeContext* ctx)
    {
        Registrar r{ctx, "__native__b2d_"};
        r("shape_create_circle",      &nShapeCreateCircle)
         ("shape_create_box",         &nShapeCreateBox)
         ("shape_create_polygon",     &nShapeCreatePolygon)
         ("shape_create_capsule",     &nShapeCreateCapsule)
         ("shape_create_segment",     &nShapeCreateSegment)
         ("chain_create",             &nChainCreate)
         ("chain_destroy",            &nChainDestroy)
         ("chain_get_segments",       &nChainGetSegments)
         ("shape_destroy",            &nShapeDestroy)
         ("shape_get_type",           &nShapeGetType)
         ("shape_get_body",           &nShapeGetBody)
         ("shape_set_density",        &nShapeSetDensity)
         ("shape_set_friction",       &nShapeset_friction)
         ("shape_set_restitution",    &nShapeset_restitution)
         ("shape_get_density",        &nShapeget_density)
         ("shape_get_friction",       &nShapeget_friction)
         ("shape_get_restitution",    &nShapeget_restitution)
         ("shape_is_sensor",          &nShapeIsSensor)
         ("shape_set_filter",         &nShapeSetFilter)
         ("shape_get_filter",         &nShapeGetFilter)
         ("shape_enable_sensor_events",  &nShapeenable_sensor_events)
         ("shape_enable_contact_events", &nShapeenable_contact_events)
         ("shape_enable_hit_events",     &nShapeenable_hit_events)
         ("shape_enable_pre_solve_events", &nShapeenable_pre_solve_events)
         ("shape_get_aabb",           &nShapeGetAABB)
         ("shape_get_circle",         &nShapeGetCircle)
         ("shape_get_segment",        &nShapeGetSegment)
         ("shape_get_polygon",        &nShapeGetPolygon)
         ("shape_get_capsule",        &nShapeGetCapsule)
         ("shape_test_point",         &nShapeTestPoint);
    }
}
