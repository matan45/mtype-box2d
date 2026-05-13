/*
 * Event iterators. The actual snapshot/refill of the per-world event vectors
 * happens at the end of world_step in WorldBindings.cpp; here we expose
 * pop-one-at-a-time natives that advance a per-event-type cursor.
 *
 * Empty array -> drained. Scripts loop:
 *   int[] e = world.nextContactBegin();
 *   while (e.length > 0) { ...; e = world.nextContactBegin(); }
 */

#include "PluginGlobals.hpp"
#include "BindingHelpers.hpp"
#include "WorldState.hpp"

#include "box2d/box2d.h"
#include "box2d/math_functions.h"

namespace mtypeb2d
{
    namespace
    {
        constexpr const char* kEx = "Box2DError";
        using namespace detail;

        WorldState* getWS(MTypeContext* ctx, int64_t key, const char* op)
        {
            WorldState* ws = findWorldState(key);
            if (!ws) {
                std::string m = std::string(op) + ": unknown world id " + std::to_string(key);
                g_host->raiseError(ctx, kEx, m.c_str());
            }
            return ws;
        }

        /* ---- contact begin: returns [shapeIdA, shapeIdB] or empty ---- */
        MTypeValue* nNextContactBegin(void*, MTypeContext* ctx,
                                      const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_world_next_contact_begin", kEx))
                return emptyIntArray(ctx);
            WorldState* ws = getWS(ctx, g_host->getInt(args[0]),
                                   "__native__b2d_world_next_contact_begin");
            if (!ws) return emptyIntArray(ctx);
            if (ws->cbCursor >= ws->contactBegin.size()) return emptyIntArray(ctx);
            const auto& e = ws->contactBegin[ws->cbCursor++];
            return makeIntPair(ctx, shapeToInt(e.shapeIdA), shapeToInt(e.shapeIdB));
        }

        /* ---- contact end: [shapeIdA, shapeIdB] ---- */
        MTypeValue* nNextContactEnd(void*, MTypeContext* ctx,
                                    const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_world_next_contact_end", kEx))
                return emptyIntArray(ctx);
            WorldState* ws = getWS(ctx, g_host->getInt(args[0]),
                                   "__native__b2d_world_next_contact_end");
            if (!ws) return emptyIntArray(ctx);
            if (ws->ceCursor >= ws->contactEnd.size()) return emptyIntArray(ctx);
            const auto& e = ws->contactEnd[ws->ceCursor++];
            return makeIntPair(ctx, shapeToInt(e.shapeIdA), shapeToInt(e.shapeIdB));
        }

        /* ---- contact hit: [shapeIdA, shapeIdB, px, py, nx, ny, approachSpeed] ---- */
        MTypeValue* nNextContactHit(void*, MTypeContext* ctx,
                                    const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_world_next_contact_hit", kEx))
                return emptyFloatArray(ctx);
            WorldState* ws = getWS(ctx, g_host->getInt(args[0]),
                                   "__native__b2d_world_next_contact_hit");
            if (!ws) return emptyFloatArray(ctx);
            if (ws->chCursor >= ws->contactHit.size()) return emptyFloatArray(ctx);
            const auto& e = ws->contactHit[ws->chCursor++];
            MTypeValue* out = g_host->makeArray(ctx, MT_TAG_FLOAT, 7);
            g_host->arraySet(out, 0, g_host->makeFloat(ctx, static_cast<double>(shapeToInt(e.shapeIdA))));
            g_host->arraySet(out, 1, g_host->makeFloat(ctx, static_cast<double>(shapeToInt(e.shapeIdB))));
            g_host->arraySet(out, 2, g_host->makeFloat(ctx, e.point.x));
            g_host->arraySet(out, 3, g_host->makeFloat(ctx, e.point.y));
            g_host->arraySet(out, 4, g_host->makeFloat(ctx, e.normal.x));
            g_host->arraySet(out, 5, g_host->makeFloat(ctx, e.normal.y));
            g_host->arraySet(out, 6, g_host->makeFloat(ctx, e.approachSpeed));
            return out;
        }

        /* ---- sensor begin: [sensorShapeId, visitorShapeId] ---- */
        MTypeValue* nNextSensorBegin(void*, MTypeContext* ctx,
                                     const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_world_next_sensor_begin", kEx))
                return emptyIntArray(ctx);
            WorldState* ws = getWS(ctx, g_host->getInt(args[0]),
                                   "__native__b2d_world_next_sensor_begin");
            if (!ws) return emptyIntArray(ctx);
            if (ws->sbCursor >= ws->sensorBegin.size()) return emptyIntArray(ctx);
            const auto& e = ws->sensorBegin[ws->sbCursor++];
            return makeIntPair(ctx, shapeToInt(e.sensorShapeId), shapeToInt(e.visitorShapeId));
        }

        MTypeValue* nNextSensorEnd(void*, MTypeContext* ctx,
                                   const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_world_next_sensor_end", kEx))
                return emptyIntArray(ctx);
            WorldState* ws = getWS(ctx, g_host->getInt(args[0]),
                                   "__native__b2d_world_next_sensor_end");
            if (!ws) return emptyIntArray(ctx);
            if (ws->seCursor >= ws->sensorEnd.size()) return emptyIntArray(ctx);
            const auto& e = ws->sensorEnd[ws->seCursor++];
            return makeIntPair(ctx, shapeToInt(e.sensorShapeId), shapeToInt(e.visitorShapeId));
        }

        /* ---- body move events: [bodyId, px, py, cos, sin, fellAsleep] ---- */
        MTypeValue* nNextBodyMove(void*, MTypeContext* ctx,
                                  const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_world_next_body_move", kEx))
                return emptyFloatArray(ctx);
            WorldState* ws = getWS(ctx, g_host->getInt(args[0]),
                                   "__native__b2d_world_next_body_move");
            if (!ws) return emptyFloatArray(ctx);
            if (ws->bmCursor >= ws->bodyMove.size()) return emptyFloatArray(ctx);
            const auto& e = ws->bodyMove[ws->bmCursor++];
            MTypeValue* out = g_host->makeArray(ctx, MT_TAG_FLOAT, 6);
            g_host->arraySet(out, 0, g_host->makeFloat(ctx, static_cast<double>(bodyToInt(e.bodyId))));
            g_host->arraySet(out, 1, g_host->makeFloat(ctx, e.transform.p.x));
            g_host->arraySet(out, 2, g_host->makeFloat(ctx, e.transform.p.y));
            g_host->arraySet(out, 3, g_host->makeFloat(ctx, e.transform.q.c));
            g_host->arraySet(out, 4, g_host->makeFloat(ctx, e.transform.q.s));
            g_host->arraySet(out, 5, g_host->makeFloat(ctx, e.fellAsleep ? 1.0 : 0.0));
            return out;
        }

        /* ---- counts ---- */

        MTypeValue* nEventCounts(void*, MTypeContext* ctx,
                                 const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_world_event_counts", kEx))
                return emptyIntArray(ctx);
            WorldState* ws = getWS(ctx, g_host->getInt(args[0]),
                                   "__native__b2d_world_event_counts");
            if (!ws) return emptyIntArray(ctx);
            MTypeValue* out = g_host->makeArray(ctx, MT_TAG_INT, 6);
            g_host->arraySet(out, 0, g_host->makeInt(ctx, static_cast<int64_t>(ws->contactBegin.size())));
            g_host->arraySet(out, 1, g_host->makeInt(ctx, static_cast<int64_t>(ws->contactEnd.size())));
            g_host->arraySet(out, 2, g_host->makeInt(ctx, static_cast<int64_t>(ws->contactHit.size())));
            g_host->arraySet(out, 3, g_host->makeInt(ctx, static_cast<int64_t>(ws->sensorBegin.size())));
            g_host->arraySet(out, 4, g_host->makeInt(ctx, static_cast<int64_t>(ws->sensorEnd.size())));
            g_host->arraySet(out, 5, g_host->makeInt(ctx, static_cast<int64_t>(ws->bodyMove.size())));
            return out;
        }

        MTypeValue* nResetCursors(void*, MTypeContext* ctx,
                                  const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_world_reset_event_cursors", kEx))
                return g_host->makeVoid(ctx);
            WorldState* ws = getWS(ctx, g_host->getInt(args[0]),
                                   "__native__b2d_world_reset_event_cursors");
            if (!ws) return g_host->makeVoid(ctx);
            ws->cbCursor = ws->ceCursor = ws->chCursor = 0;
            ws->sbCursor = ws->seCursor = ws->bmCursor = 0;
            return g_host->makeVoid(ctx);
        }
    }

    void registerEventNatives(MTypeContext* ctx)
    {
        Registrar r{ctx, "__native__b2d_"};
        r("world_next_contact_begin", &nNextContactBegin)
         ("world_next_contact_end",   &nNextContactEnd)
         ("world_next_contact_hit",   &nNextContactHit)
         ("world_next_sensor_begin",  &nNextSensorBegin)
         ("world_next_sensor_end",    &nNextSensorEnd)
         ("world_next_body_move",     &nNextBodyMove)
         ("world_event_counts",       &nEventCounts)
         ("world_reset_event_cursors", &nResetCursors);
    }
}
