/*
 * Small math helpers for mType code. b2Vec2 / b2Rot / b2Transform are not
 * directly exposed — we just give scripts radians/degrees helpers and
 * cos/sin pair construction.
 */

#include "PluginGlobals.hpp"
#include "BindingHelpers.hpp"

#include "box2d/math_functions.h"

namespace mtypeb2d
{
    namespace
    {
        constexpr const char* kEx = "Box2DError";
        using namespace detail;

        constexpr double DEG_TO_RAD = 0.017453292519943295;
        constexpr double RAD_TO_DEG = 57.29577951308232;

        MTypeValue* nRadiansFromDegrees(void*, MTypeContext* ctx,
                                        const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_math_radians_from_degrees", kEx))
                return g_host->makeFloat(ctx, 0.0);
            return g_host->makeFloat(ctx, g_host->getFloat(args[0]) * DEG_TO_RAD);
        }

        MTypeValue* nDegreesFromRadians(void*, MTypeContext* ctx,
                                        const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_math_degrees_from_radians", kEx))
                return g_host->makeFloat(ctx, 0.0);
            return g_host->makeFloat(ctx, g_host->getFloat(args[0]) * RAD_TO_DEG);
        }

        MTypeValue* nRotFromAngle(void*, MTypeContext* ctx,
                                  const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_math_rot_from_angle", kEx))
                return emptyFloatArray(ctx);
            b2Rot r = b2MakeRot(getF(args[0]));
            return makeFloatPair(ctx, r.c, r.s);
        }
    }

    void registerMathNatives(MTypeContext* ctx)
    {
        Registrar r{ctx, "__native__b2d_"};
        r("math_radians_from_degrees", &nRadiansFromDegrees)
         ("math_degrees_from_radians", &nDegreesFromRadians)
         ("math_rot_from_angle",       &nRotFromAngle);
    }
}
