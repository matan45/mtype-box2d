/*
 * Query bindings: ray cast and AABB overlap.
 *
 * Box2D v3's b2World_OverlapAABB / b2World_CastRay take a result callback
 * (b2OverlapResultFcn / b2CastResultFcn) and a void* context. Both queries
 * are synchronous (the callback fires inside the call), so capturing a
 * stack-local std::vector* through the context is safe.
 *
 * Closest-hit ray casts use b2World_CastRayClosest (no callback needed).
 */

#include "PluginGlobals.hpp"
#include "BindingHelpers.hpp"

#include "box2d/box2d.h"
#include "box2d/math_functions.h"
#include "box2d/types.h"

#include <vector>

namespace mtypeb2d
{
    namespace
    {
        constexpr const char* kEx = "Box2DError";
        using namespace detail;

        /* OverlapAABB context: a vector of shape ids found by the query. */
        struct OverlapCtx { std::vector<b2ShapeId>* out; };
        bool overlapCb(b2ShapeId id, void* ctx)
        {
            static_cast<OverlapCtx*>(ctx)->out->push_back(id);
            return true; /* continue */
        }

        /* CastRay-all context: tuples of (shapeId, point, normal, fraction). */
        struct RayAllCtx
        {
            std::vector<int64_t>* shapeIds;
            std::vector<float>*   floats; /* px, py, nx, ny, fraction per hit */
        };
        float rayAllCb(b2ShapeId id, b2Vec2 point, b2Vec2 normal, float fraction, void* ctx)
        {
            auto* c = static_cast<RayAllCtx*>(ctx);
            c->shapeIds->push_back(shapeToInt(id));
            c->floats->push_back(point.x);
            c->floats->push_back(point.y);
            c->floats->push_back(normal.x);
            c->floats->push_back(normal.y);
            c->floats->push_back(fraction);
            return 1.0f; /* continue, don't clip */
        }

        /* ---- world overlap aabb ---- */

        MTypeValue* nWorldOverlapAABB(void*, MTypeContext* ctx,
                                      const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 7, "__native__b2d_world_overlap_aabb", kEx))
                return emptyIntArray(ctx);
            b2WorldId world = intToWorld(g_host->getInt(args[0]));
            if (!validateWorld(world, ctx, "__native__b2d_world_overlap_aabb", kEx))
                return emptyIntArray(ctx);
            b2AABB aabb;
            aabb.lowerBound = getVec2(args[1], args[2]);
            aabb.upperBound = getVec2(args[3], args[4]);
            b2QueryFilter f;
            f.categoryBits = getU64(args[5]);
            f.maskBits     = getU64(args[6]);

            std::vector<b2ShapeId> hits;
            OverlapCtx oc{ &hits };
            b2World_OverlapAABB(world, aabb, f, &overlapCb, &oc);

            MTypeValue* out = g_host->makeArray(ctx, MT_TAG_INT, hits.size());
            for (size_t i = 0; i < hits.size(); ++i)
                g_host->arraySet(out, i, g_host->makeInt(ctx, shapeToInt(hits[i])));
            return out;
        }

        /* ---- world cast ray (closest) ---- */

        MTypeValue* nWorldCastRayClosest(void*, MTypeContext* ctx,
                                         const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 7, "__native__b2d_world_cast_ray_closest", kEx))
                return emptyFloatArray(ctx);
            b2WorldId world = intToWorld(g_host->getInt(args[0]));
            if (!validateWorld(world, ctx, "__native__b2d_world_cast_ray_closest", kEx))
                return emptyFloatArray(ctx);
            b2Vec2 origin      = getVec2(args[1], args[2]);
            b2Vec2 translation = getVec2(args[3], args[4]);
            b2QueryFilter f;
            f.categoryBits = getU64(args[5]);
            f.maskBits     = getU64(args[6]);

            b2RayResult r = b2World_CastRayClosest(world, origin, translation, f);
            if (!r.hit) return emptyFloatArray(ctx);

            /* Return: [hit=1, shapeId, px, py, nx, ny, fraction] */
            MTypeValue* out = g_host->makeArray(ctx, MT_TAG_FLOAT, 7);
            g_host->arraySet(out, 0, g_host->makeFloat(ctx, 1.0));
            g_host->arraySet(out, 1, g_host->makeFloat(ctx, static_cast<double>(shapeToInt(r.shapeId))));
            g_host->arraySet(out, 2, g_host->makeFloat(ctx, r.point.x));
            g_host->arraySet(out, 3, g_host->makeFloat(ctx, r.point.y));
            g_host->arraySet(out, 4, g_host->makeFloat(ctx, r.normal.x));
            g_host->arraySet(out, 5, g_host->makeFloat(ctx, r.normal.y));
            g_host->arraySet(out, 6, g_host->makeFloat(ctx, r.fraction));
            return out;
        }

        /* ---- world cast ray (all hits) ---- */

        MTypeValue* nWorldCastRayAll(void*, MTypeContext* ctx,
                                     const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 7, "__native__b2d_world_cast_ray_all", kEx))
                return emptyFloatArray(ctx);
            b2WorldId world = intToWorld(g_host->getInt(args[0]));
            if (!validateWorld(world, ctx, "__native__b2d_world_cast_ray_all", kEx))
                return emptyFloatArray(ctx);
            b2Vec2 origin      = getVec2(args[1], args[2]);
            b2Vec2 translation = getVec2(args[3], args[4]);
            b2QueryFilter f;
            f.categoryBits = getU64(args[5]);
            f.maskBits     = getU64(args[6]);

            std::vector<int64_t> shapeIds;
            std::vector<float>   floats;
            RayAllCtx rc{ &shapeIds, &floats };
            b2World_CastRay(world, origin, translation, f, &rayAllCb, &rc);

            /* Flatten: [n, sid0, px0, py0, nx0, ny0, frac0, sid1, ...] */
            size_t n = shapeIds.size();
            MTypeValue* out = g_host->makeArray(ctx, MT_TAG_FLOAT, 1 + 6 * n);
            g_host->arraySet(out, 0, g_host->makeFloat(ctx, static_cast<double>(n)));
            for (size_t i = 0; i < n; ++i) {
                size_t base = 1 + 6 * i;
                g_host->arraySet(out, base + 0, g_host->makeFloat(ctx, static_cast<double>(shapeIds[i])));
                g_host->arraySet(out, base + 1, g_host->makeFloat(ctx, floats[5 * i + 0]));
                g_host->arraySet(out, base + 2, g_host->makeFloat(ctx, floats[5 * i + 1]));
                g_host->arraySet(out, base + 3, g_host->makeFloat(ctx, floats[5 * i + 2]));
                g_host->arraySet(out, base + 4, g_host->makeFloat(ctx, floats[5 * i + 3]));
                g_host->arraySet(out, base + 5, g_host->makeFloat(ctx, floats[5 * i + 4]));
            }
            return out;
        }
    }

    void registerQueryNatives(MTypeContext* ctx)
    {
        Registrar r{ctx, "__native__b2d_"};
        r("world_overlap_aabb",      &nWorldOverlapAABB)
         ("world_cast_ray_closest",  &nWorldCastRayClosest)
         ("world_cast_ray_all",      &nWorldCastRayAll);
    }
}
