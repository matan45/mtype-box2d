/*
 * Box2D world lifecycle + step + per-world settings.
 *
 * Step is the critical native: at the end of each step it snapshots the
 * world's event buffers into the per-world WorldState (so mType can drain
 * events lazily; see EventBindings.cpp), and resets the cursors.
 */

#include "PluginGlobals.hpp"
#include "BindingHelpers.hpp"
#include "WorldState.hpp"

#include "box2d/box2d.h"
#include "box2d/types.h"

namespace mtypeb2d
{
    namespace
    {
        constexpr const char* kEx = "Box2DError";
        using namespace detail;

        /* Snapshot the world's contact/sensor/body event arrays into the
         * WorldState. Called from nWorldStep right after b2World_Step so the
         * native arrays (which Box2D wipes on the next step) are safely
         * copied to plugin-owned storage. */
        void snapshotEvents(b2WorldId worldId, WorldState& ws)
        {
            auto ce = b2World_GetContactEvents(worldId);
            ws.contactBegin.assign(ce.beginEvents, ce.beginEvents + ce.beginCount);
            ws.contactEnd  .assign(ce.endEvents,   ce.endEvents   + ce.endCount);
            ws.contactHit  .assign(ce.hitEvents,   ce.hitEvents   + ce.hitCount);

            auto se = b2World_GetSensorEvents(worldId);
            ws.sensorBegin.assign(se.beginEvents, se.beginEvents + se.beginCount);
            ws.sensorEnd  .assign(se.endEvents,   se.endEvents   + se.endCount);

            auto be = b2World_GetBodyEvents(worldId);
            ws.bodyMove.assign(be.moveEvents, be.moveEvents + be.moveCount);

            ws.cbCursor = ws.ceCursor = ws.chCursor = 0;
            ws.sbCursor = ws.seCursor = ws.bmCursor = 0;
        }

        /* ---- world lifecycle ---- */

        MTypeValue* nWorldCreate(void*, MTypeContext* ctx,
                                 const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 2, "__native__b2d_world_create", kEx))
                return g_host->makeInt(ctx, 0);
            b2WorldDef def = b2DefaultWorldDef();
            def.gravity.x = getF(args[0]);
            def.gravity.y = getF(args[1]);
            b2WorldId id = b2CreateWorld(&def);
            int64_t key = worldToInt(id);
            createWorldState(key);
            return g_host->makeInt(ctx, key);
        }

        MTypeValue* nWorldCreateFromDef(void*, MTypeContext* ctx,
                                        const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_world_create_from_def", kEx))
                return g_host->makeInt(ctx, 0);
            auto* d = findDefOrRaise(g_worldDefs, g_host->getInt(args[0]), ctx,
                                     "__native__b2d_world_create_from_def", kEx);
            if (!d) return g_host->makeInt(ctx, 0);
            b2WorldId id = b2CreateWorld(d);
            int64_t key = worldToInt(id);
            createWorldState(key);
            return g_host->makeInt(ctx, key);
        }

        MTypeValue* nWorldDestroy(void*, MTypeContext* ctx,
                                  const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_world_destroy", kEx))
                return g_host->makeVoid(ctx);
            int64_t key = g_host->getInt(args[0]);
            b2WorldId id = intToWorld(key);
            if (b2World_IsValid(id)) b2DestroyWorld(id);
            destroyWorldState(key);
            return g_host->makeVoid(ctx);
        }

        MTypeValue* nWorldStep(void*, MTypeContext* ctx,
                               const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 3, "__native__b2d_world_step", kEx))
                return g_host->makeVoid(ctx);
            b2WorldId id = intToWorld(g_host->getInt(args[0]));
            if (!validateWorld(id, ctx, "__native__b2d_world_step", kEx))
                return g_host->makeVoid(ctx);

            float dt = getF(args[1]);
            int sub  = getI(args[2]);
            b2World_Step(id, dt, sub);

            WorldState* ws = findWorldState(g_host->getInt(args[0]));
            if (ws) snapshotEvents(id, *ws);
            return g_host->makeVoid(ctx);
        }

        /* ---- gravity & misc settings ---- */

        MTypeValue* nWorldSetGravity(void*, MTypeContext* ctx,
                                     const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 3, "__native__b2d_world_set_gravity", kEx))
                return g_host->makeVoid(ctx);
            b2WorldId id = intToWorld(g_host->getInt(args[0]));
            if (!validateWorld(id, ctx, "__native__b2d_world_set_gravity", kEx))
                return g_host->makeVoid(ctx);
            b2World_SetGravity(id, getVec2(args[1], args[2]));
            return g_host->makeVoid(ctx);
        }

        MTypeValue* nWorldGetGravity(void*, MTypeContext* ctx,
                                     const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_world_get_gravity", kEx))
                return emptyFloatArray(ctx);
            b2WorldId id = intToWorld(g_host->getInt(args[0]));
            if (!validateWorld(id, ctx, "__native__b2d_world_get_gravity", kEx))
                return emptyFloatArray(ctx);
            b2Vec2 g = b2World_GetGravity(id);
            return makeFloatPair(ctx, g.x, g.y);
        }

        MTypeValue* nWorldEnableSleeping(void*, MTypeContext* ctx,
                                         const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 2, "__native__b2d_world_enable_sleeping", kEx))
                return g_host->makeVoid(ctx);
            b2WorldId id = intToWorld(g_host->getInt(args[0]));
            if (!validateWorld(id, ctx, "__native__b2d_world_enable_sleeping", kEx))
                return g_host->makeVoid(ctx);
            b2World_EnableSleeping(id, getB(args[1]));
            return g_host->makeVoid(ctx);
        }

        MTypeValue* nWorldEnableContinuous(void*, MTypeContext* ctx,
                                           const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 2, "__native__b2d_world_enable_continuous", kEx))
                return g_host->makeVoid(ctx);
            b2WorldId id = intToWorld(g_host->getInt(args[0]));
            if (!validateWorld(id, ctx, "__native__b2d_world_enable_continuous", kEx))
                return g_host->makeVoid(ctx);
            b2World_EnableContinuous(id, getB(args[1]));
            return g_host->makeVoid(ctx);
        }

        MTypeValue* nWorldSetRestitutionThreshold(void*, MTypeContext* ctx,
                                                  const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 2, "__native__b2d_world_set_restitution_threshold", kEx))
                return g_host->makeVoid(ctx);
            b2WorldId id = intToWorld(g_host->getInt(args[0]));
            if (!validateWorld(id, ctx, "__native__b2d_world_set_restitution_threshold", kEx))
                return g_host->makeVoid(ctx);
            b2World_SetRestitutionThreshold(id, getF(args[1]));
            return g_host->makeVoid(ctx);
        }

        MTypeValue* nWorldSetHitEventThreshold(void*, MTypeContext* ctx,
                                               const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 2, "__native__b2d_world_set_hit_event_threshold", kEx))
                return g_host->makeVoid(ctx);
            b2WorldId id = intToWorld(g_host->getInt(args[0]));
            if (!validateWorld(id, ctx, "__native__b2d_world_set_hit_event_threshold", kEx))
                return g_host->makeVoid(ctx);
            b2World_SetHitEventThreshold(id, getF(args[1]));
            return g_host->makeVoid(ctx);
        }

        MTypeValue* nWorldGetAwakeBodyCount(void*, MTypeContext* ctx,
                                            const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_world_get_awake_body_count", kEx))
                return g_host->makeInt(ctx, 0);
            b2WorldId id = intToWorld(g_host->getInt(args[0]));
            if (!validateWorld(id, ctx, "__native__b2d_world_get_awake_body_count", kEx))
                return g_host->makeInt(ctx, 0);
            return g_host->makeInt(ctx, b2World_GetAwakeBodyCount(id));
        }

        MTypeValue* nWorldExplode(void*, MTypeContext* ctx,
                                  const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 6, "__native__b2d_world_explode", kEx))
                return g_host->makeVoid(ctx);
            b2WorldId id = intToWorld(g_host->getInt(args[0]));
            if (!validateWorld(id, ctx, "__native__b2d_world_explode", kEx))
                return g_host->makeVoid(ctx);
            b2ExplosionDef def = b2DefaultExplosionDef();
            def.position.x      = getF(args[1]);
            def.position.y      = getF(args[2]);
            def.radius          = getF(args[3]);
            def.falloff         = getF(args[4]);
            def.impulsePerLength = getF(args[5]);
            b2World_Explode(id, &def);
            return g_host->makeVoid(ctx);
        }
    }

    void registerWorldNatives(MTypeContext* ctx)
    {
        Registrar r{ctx, "__native__b2d_"};
        r("world_create",                     &nWorldCreate)
         ("world_create_from_def",            &nWorldCreateFromDef)
         ("world_destroy",                    &nWorldDestroy)
         ("world_step",                       &nWorldStep)
         ("world_set_gravity",                &nWorldSetGravity)
         ("world_get_gravity",                &nWorldGetGravity)
         ("world_enable_sleeping",            &nWorldEnableSleeping)
         ("world_enable_continuous",          &nWorldEnableContinuous)
         ("world_set_restitution_threshold",  &nWorldSetRestitutionThreshold)
         ("world_set_hit_event_threshold",    &nWorldSetHitEventThreshold)
         ("world_get_awake_body_count",       &nWorldGetAwakeBodyCount)
         ("world_explode",                    &nWorldExplode);
    }
}
