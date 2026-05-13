/*
 * Joint creation + per-type runtime setters/getters.
 *
 * Each joint is created from a per-type def handle (built via the *_joint_def_*
 * natives in DefBindings.cpp). After creation the def handle remains valid;
 * mType code is expected to call def.destroy() to release it.
 */

#include "PluginGlobals.hpp"
#include "BindingHelpers.hpp"

#include "box2d/box2d.h"
#include "box2d/types.h"

namespace mtypeb2d
{
    namespace
    {
        constexpr const char* kEx = "Box2DError";
        using namespace detail;

        /* ---- factories ---- */

        #define JOINT_FACTORY(PREFIX, TYPE, REG, FN) \
        MTypeValue* nCreate_##PREFIX(void*, MTypeContext* ctx, \
                                     const MTypeValue* const* args, int argc) \
        { \
            if (!requireArgs(ctx, argc, 2, "__native__b2d_" #PREFIX "_joint_create", kEx)) \
                return g_host->makeInt(ctx, 0); \
            b2WorldId world = intToWorld(g_host->getInt(args[0])); \
            if (!validateWorld(world, ctx, "__native__b2d_" #PREFIX "_joint_create", kEx)) \
                return g_host->makeInt(ctx, 0); \
            auto* def = findDefOrRaise(REG, g_host->getInt(args[1]), ctx, \
                                       "__native__b2d_" #PREFIX "_joint_create", kEx); \
            if (!def) return g_host->makeInt(ctx, 0); \
            b2JointId id = FN(world, def); \
            return g_host->makeInt(ctx, jointToInt(id)); \
        }

        JOINT_FACTORY(distance,  b2DistanceJointDef,  g_distanceJointDefs,  b2CreateDistanceJoint)
        JOINT_FACTORY(revolute,  b2RevoluteJointDef,  g_revoluteJointDefs,  b2CreateRevoluteJoint)
        JOINT_FACTORY(prismatic, b2PrismaticJointDef, g_prismaticJointDefs, b2CreatePrismaticJoint)
        JOINT_FACTORY(weld,      b2WeldJointDef,      g_weldJointDefs,      b2CreateWeldJoint)
        JOINT_FACTORY(motor,     b2MotorJointDef,     g_motorJointDefs,     b2CreateMotorJoint)
        JOINT_FACTORY(mouse,     b2MouseJointDef,     g_mouseJointDefs,     b2CreateMouseJoint)
        JOINT_FACTORY(wheel,     b2WheelJointDef,     g_wheelJointDefs,     b2CreateWheelJoint)
        JOINT_FACTORY(filter,    b2FilterJointDef,    g_filterJointDefs,    b2CreateFilterJoint)

        #undef JOINT_FACTORY

        /* ---- common joint ops ---- */

        MTypeValue* nJointDestroy(void*, MTypeContext* ctx,
                                  const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_joint_destroy", kEx))
                return g_host->makeVoid(ctx);
            b2JointId id = intToJoint(g_host->getInt(args[0]));
            if (b2Joint_IsValid(id)) b2DestroyJoint(id);
            return g_host->makeVoid(ctx);
        }

        MTypeValue* nJointGetType(void*, MTypeContext* ctx,
                                  const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_joint_get_type", kEx))
                return g_host->makeInt(ctx, 0);
            b2JointId id = intToJoint(g_host->getInt(args[0]));
            if (!validateJoint(id, ctx, "__native__b2d_joint_get_type", kEx))
                return g_host->makeInt(ctx, 0);
            return g_host->makeInt(ctx, static_cast<int>(b2Joint_GetType(id)));
        }

        MTypeValue* nJointGetBodies(void*, MTypeContext* ctx,
                                    const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_joint_get_bodies", kEx))
                return emptyIntArray(ctx);
            b2JointId id = intToJoint(g_host->getInt(args[0]));
            if (!validateJoint(id, ctx, "__native__b2d_joint_get_bodies", kEx))
                return emptyIntArray(ctx);
            return makeIntPair(ctx,
                bodyToInt(b2Joint_GetBodyA(id)),
                bodyToInt(b2Joint_GetBodyB(id)));
        }

        MTypeValue* nJointGetLocalAnchorA(void*, MTypeContext* ctx,
                                          const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_joint_get_local_anchor_a", kEx))
                return emptyFloatArray(ctx);
            b2JointId id = intToJoint(g_host->getInt(args[0]));
            if (!validateJoint(id, ctx, "__native__b2d_joint_get_local_anchor_a", kEx))
                return emptyFloatArray(ctx);
            b2Vec2 v = b2Joint_GetLocalAnchorA(id);
            return makeFloatPair(ctx, v.x, v.y);
        }

        MTypeValue* nJointGetLocalAnchorB(void*, MTypeContext* ctx,
                                          const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_joint_get_local_anchor_b", kEx))
                return emptyFloatArray(ctx);
            b2JointId id = intToJoint(g_host->getInt(args[0]));
            if (!validateJoint(id, ctx, "__native__b2d_joint_get_local_anchor_b", kEx))
                return emptyFloatArray(ctx);
            b2Vec2 v = b2Joint_GetLocalAnchorB(id);
            return makeFloatPair(ctx, v.x, v.y);
        }

        MTypeValue* nJointGetConstraintForce(void*, MTypeContext* ctx,
                                             const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_joint_get_constraint_force", kEx))
                return emptyFloatArray(ctx);
            b2JointId id = intToJoint(g_host->getInt(args[0]));
            if (!validateJoint(id, ctx, "__native__b2d_joint_get_constraint_force", kEx))
                return emptyFloatArray(ctx);
            b2Vec2 f = b2Joint_GetConstraintForce(id);
            return makeFloatPair(ctx, f.x, f.y);
        }

        MTypeValue* nJointGetConstraintTorque(void*, MTypeContext* ctx,
                                              const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_joint_get_constraint_torque", kEx))
                return g_host->makeFloat(ctx, 0.0);
            b2JointId id = intToJoint(g_host->getInt(args[0]));
            if (!validateJoint(id, ctx, "__native__b2d_joint_get_constraint_torque", kEx))
                return g_host->makeFloat(ctx, 0.0);
            return g_host->makeFloat(ctx, b2Joint_GetConstraintTorque(id));
        }

        MTypeValue* nJointWakeBodies(void*, MTypeContext* ctx,
                                     const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_joint_wake_bodies", kEx))
                return g_host->makeVoid(ctx);
            b2JointId id = intToJoint(g_host->getInt(args[0]));
            if (!validateJoint(id, ctx, "__native__b2d_joint_wake_bodies", kEx))
                return g_host->makeVoid(ctx);
            b2Joint_WakeBodies(id);
            return g_host->makeVoid(ctx);
        }

        /* ---- per-type setters/getters ---- */

        #define JOINT_SET1F(PREFIX, NAME, FN) \
        MTypeValue* n##PREFIX##_##NAME(void*, MTypeContext* ctx, \
                                       const MTypeValue* const* args, int argc) \
        { \
            if (!requireArgs(ctx, argc, 2, "__native__b2d_" #PREFIX "_" #NAME, kEx)) \
                return g_host->makeVoid(ctx); \
            b2JointId id = intToJoint(g_host->getInt(args[0])); \
            if (!validateJoint(id, ctx, "__native__b2d_" #PREFIX "_" #NAME, kEx)) \
                return g_host->makeVoid(ctx); \
            FN(id, getF(args[1])); \
            return g_host->makeVoid(ctx); \
        }

        #define JOINT_SET1B(PREFIX, NAME, FN) \
        MTypeValue* n##PREFIX##_##NAME(void*, MTypeContext* ctx, \
                                       const MTypeValue* const* args, int argc) \
        { \
            if (!requireArgs(ctx, argc, 2, "__native__b2d_" #PREFIX "_" #NAME, kEx)) \
                return g_host->makeVoid(ctx); \
            b2JointId id = intToJoint(g_host->getInt(args[0])); \
            if (!validateJoint(id, ctx, "__native__b2d_" #PREFIX "_" #NAME, kEx)) \
                return g_host->makeVoid(ctx); \
            FN(id, getB(args[1])); \
            return g_host->makeVoid(ctx); \
        }

        #define JOINT_GET1F(PREFIX, NAME, FN) \
        MTypeValue* n##PREFIX##_##NAME(void*, MTypeContext* ctx, \
                                       const MTypeValue* const* args, int argc) \
        { \
            if (!requireArgs(ctx, argc, 1, "__native__b2d_" #PREFIX "_" #NAME, kEx)) \
                return g_host->makeFloat(ctx, 0.0); \
            b2JointId id = intToJoint(g_host->getInt(args[0])); \
            if (!validateJoint(id, ctx, "__native__b2d_" #PREFIX "_" #NAME, kEx)) \
                return g_host->makeFloat(ctx, 0.0); \
            return g_host->makeFloat(ctx, FN(id)); \
        }

        /* distance */
        JOINT_SET1F(distance_joint, set_length,         b2DistanceJoint_SetLength)
        JOINT_GET1F(distance_joint, get_length,         b2DistanceJoint_GetLength)
        JOINT_GET1F(distance_joint, get_current_length, b2DistanceJoint_GetCurrentLength)
        JOINT_SET1B(distance_joint, enable_spring,      b2DistanceJoint_EnableSpring)
        JOINT_SET1F(distance_joint, set_spring_hertz,   b2DistanceJoint_SetSpringHertz)
        JOINT_SET1F(distance_joint, set_spring_damping_ratio, b2DistanceJoint_SetSpringDampingRatio)
        JOINT_SET1B(distance_joint, enable_limit,       b2DistanceJoint_EnableLimit)
        JOINT_SET1B(distance_joint, enable_motor,       b2DistanceJoint_EnableMotor)
        JOINT_SET1F(distance_joint, set_motor_speed,    b2DistanceJoint_SetMotorSpeed)
        JOINT_SET1F(distance_joint, set_max_motor_force, b2DistanceJoint_SetMaxMotorForce)
        JOINT_GET1F(distance_joint, get_motor_force,    b2DistanceJoint_GetMotorForce)

        /* revolute */
        JOINT_GET1F(revolute_joint, get_angle,          b2RevoluteJoint_GetAngle)
        JOINT_SET1B(revolute_joint, enable_spring,      b2RevoluteJoint_EnableSpring)
        JOINT_SET1F(revolute_joint, set_spring_hertz,   b2RevoluteJoint_SetSpringHertz)
        JOINT_SET1F(revolute_joint, set_spring_damping_ratio, b2RevoluteJoint_SetSpringDampingRatio)
        JOINT_SET1B(revolute_joint, enable_limit,       b2RevoluteJoint_EnableLimit)
        JOINT_SET1B(revolute_joint, enable_motor,       b2RevoluteJoint_EnableMotor)
        JOINT_SET1F(revolute_joint, set_motor_speed,    b2RevoluteJoint_SetMotorSpeed)
        JOINT_GET1F(revolute_joint, get_motor_speed,    b2RevoluteJoint_GetMotorSpeed)
        JOINT_SET1F(revolute_joint, set_max_motor_torque, b2RevoluteJoint_SetMaxMotorTorque)
        JOINT_GET1F(revolute_joint, get_motor_torque,   b2RevoluteJoint_GetMotorTorque)

        MTypeValue* nrevolute_joint_set_limits(void*, MTypeContext* ctx,
                                               const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 3, "__native__b2d_revolute_joint_set_limits", kEx))
                return g_host->makeVoid(ctx);
            b2JointId id = intToJoint(g_host->getInt(args[0]));
            if (!validateJoint(id, ctx, "__native__b2d_revolute_joint_set_limits", kEx))
                return g_host->makeVoid(ctx);
            b2RevoluteJoint_SetLimits(id, getF(args[1]), getF(args[2]));
            return g_host->makeVoid(ctx);
        }

        /* prismatic */
        JOINT_SET1B(prismatic_joint, enable_spring,     b2PrismaticJoint_EnableSpring)
        JOINT_SET1F(prismatic_joint, set_spring_hertz,  b2PrismaticJoint_SetSpringHertz)
        JOINT_SET1F(prismatic_joint, set_spring_damping_ratio, b2PrismaticJoint_SetSpringDampingRatio)
        JOINT_SET1B(prismatic_joint, enable_limit,      b2PrismaticJoint_EnableLimit)
        JOINT_SET1B(prismatic_joint, enable_motor,      b2PrismaticJoint_EnableMotor)
        JOINT_SET1F(prismatic_joint, set_motor_speed,   b2PrismaticJoint_SetMotorSpeed)
        JOINT_SET1F(prismatic_joint, set_max_motor_force, b2PrismaticJoint_SetMaxMotorForce)
        JOINT_GET1F(prismatic_joint, get_translation,   b2PrismaticJoint_GetTranslation)
        JOINT_GET1F(prismatic_joint, get_speed,         b2PrismaticJoint_GetSpeed)

        MTypeValue* nprismatic_joint_set_limits(void*, MTypeContext* ctx,
                                                const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 3, "__native__b2d_prismatic_joint_set_limits", kEx))
                return g_host->makeVoid(ctx);
            b2JointId id = intToJoint(g_host->getInt(args[0]));
            if (!validateJoint(id, ctx, "__native__b2d_prismatic_joint_set_limits", kEx))
                return g_host->makeVoid(ctx);
            b2PrismaticJoint_SetLimits(id, getF(args[1]), getF(args[2]));
            return g_host->makeVoid(ctx);
        }

        /* weld */
        JOINT_SET1F(weld_joint, set_linear_hertz,      b2WeldJoint_SetLinearHertz)
        JOINT_SET1F(weld_joint, set_angular_hertz,     b2WeldJoint_SetAngularHertz)
        JOINT_SET1F(weld_joint, set_linear_damping_ratio,  b2WeldJoint_SetLinearDampingRatio)
        JOINT_SET1F(weld_joint, set_angular_damping_ratio, b2WeldJoint_SetAngularDampingRatio)
        JOINT_SET1F(weld_joint, set_reference_angle,   b2WeldJoint_SetReferenceAngle)
        JOINT_GET1F(weld_joint, get_reference_angle,   b2WeldJoint_GetReferenceAngle)

        /* motor */
        MTypeValue* nmotor_joint_set_linear_offset(void*, MTypeContext* ctx,
                                                   const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 3, "__native__b2d_motor_joint_set_linear_offset", kEx))
                return g_host->makeVoid(ctx);
            b2JointId id = intToJoint(g_host->getInt(args[0]));
            if (!validateJoint(id, ctx, "__native__b2d_motor_joint_set_linear_offset", kEx))
                return g_host->makeVoid(ctx);
            b2MotorJoint_SetLinearOffset(id, getVec2(args[1], args[2]));
            return g_host->makeVoid(ctx);
        }
        JOINT_SET1F(motor_joint, set_angular_offset,   b2MotorJoint_SetAngularOffset)
        JOINT_SET1F(motor_joint, set_max_force,        b2MotorJoint_SetMaxForce)
        JOINT_SET1F(motor_joint, set_max_torque,       b2MotorJoint_SetMaxTorque)
        JOINT_SET1F(motor_joint, set_correction_factor, b2MotorJoint_SetCorrectionFactor)

        /* mouse */
        MTypeValue* nmouse_joint_set_target(void*, MTypeContext* ctx,
                                            const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 3, "__native__b2d_mouse_joint_set_target", kEx))
                return g_host->makeVoid(ctx);
            b2JointId id = intToJoint(g_host->getInt(args[0]));
            if (!validateJoint(id, ctx, "__native__b2d_mouse_joint_set_target", kEx))
                return g_host->makeVoid(ctx);
            b2MouseJoint_SetTarget(id, getVec2(args[1], args[2]));
            return g_host->makeVoid(ctx);
        }
        JOINT_SET1F(mouse_joint, set_spring_hertz,     b2MouseJoint_SetSpringHertz)
        JOINT_SET1F(mouse_joint, set_spring_damping_ratio, b2MouseJoint_SetSpringDampingRatio)
        JOINT_SET1F(mouse_joint, set_max_force,        b2MouseJoint_SetMaxForce)

        /* wheel */
        JOINT_SET1B(wheel_joint, enable_spring,        b2WheelJoint_EnableSpring)
        JOINT_SET1F(wheel_joint, set_spring_hertz,     b2WheelJoint_SetSpringHertz)
        JOINT_SET1F(wheel_joint, set_spring_damping_ratio, b2WheelJoint_SetSpringDampingRatio)
        JOINT_SET1B(wheel_joint, enable_limit,         b2WheelJoint_EnableLimit)
        JOINT_SET1B(wheel_joint, enable_motor,         b2WheelJoint_EnableMotor)
        JOINT_SET1F(wheel_joint, set_motor_speed,      b2WheelJoint_SetMotorSpeed)
        JOINT_SET1F(wheel_joint, set_max_motor_torque, b2WheelJoint_SetMaxMotorTorque)
        JOINT_GET1F(wheel_joint, get_motor_torque,     b2WheelJoint_GetMotorTorque)

        MTypeValue* nwheel_joint_set_limits(void*, MTypeContext* ctx,
                                            const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 3, "__native__b2d_wheel_joint_set_limits", kEx))
                return g_host->makeVoid(ctx);
            b2JointId id = intToJoint(g_host->getInt(args[0]));
            if (!validateJoint(id, ctx, "__native__b2d_wheel_joint_set_limits", kEx))
                return g_host->makeVoid(ctx);
            b2WheelJoint_SetLimits(id, getF(args[1]), getF(args[2]));
            return g_host->makeVoid(ctx);
        }

        #undef JOINT_SET1F
        #undef JOINT_SET1B
        #undef JOINT_GET1F
    }

    void registerJointNatives(MTypeContext* ctx)
    {
        Registrar r{ctx, "__native__b2d_"};

        /* factories */
        r("distance_joint_create",  &nCreate_distance)
         ("revolute_joint_create",  &nCreate_revolute)
         ("prismatic_joint_create", &nCreate_prismatic)
         ("weld_joint_create",      &nCreate_weld)
         ("motor_joint_create",     &nCreate_motor)
         ("mouse_joint_create",     &nCreate_mouse)
         ("wheel_joint_create",     &nCreate_wheel)
         ("filter_joint_create",    &nCreate_filter);

        /* common */
        r("joint_destroy",                  &nJointDestroy)
         ("joint_get_type",                 &nJointGetType)
         ("joint_get_bodies",               &nJointGetBodies)
         ("joint_get_local_anchor_a",       &nJointGetLocalAnchorA)
         ("joint_get_local_anchor_b",       &nJointGetLocalAnchorB)
         ("joint_get_constraint_force",     &nJointGetConstraintForce)
         ("joint_get_constraint_torque",    &nJointGetConstraintTorque)
         ("joint_wake_bodies",              &nJointWakeBodies);

        /* per-type */
        r("distance_joint_set_length",       &ndistance_joint_set_length)
         ("distance_joint_get_length",       &ndistance_joint_get_length)
         ("distance_joint_get_current_length", &ndistance_joint_get_current_length)
         ("distance_joint_enable_spring",    &ndistance_joint_enable_spring)
         ("distance_joint_set_spring_hertz", &ndistance_joint_set_spring_hertz)
         ("distance_joint_set_spring_damping_ratio", &ndistance_joint_set_spring_damping_ratio)
         ("distance_joint_enable_limit",     &ndistance_joint_enable_limit)
         ("distance_joint_enable_motor",     &ndistance_joint_enable_motor)
         ("distance_joint_set_motor_speed",  &ndistance_joint_set_motor_speed)
         ("distance_joint_set_max_motor_force", &ndistance_joint_set_max_motor_force)
         ("distance_joint_get_motor_force",  &ndistance_joint_get_motor_force);

        r("revolute_joint_get_angle",       &nrevolute_joint_get_angle)
         ("revolute_joint_enable_spring",   &nrevolute_joint_enable_spring)
         ("revolute_joint_set_spring_hertz", &nrevolute_joint_set_spring_hertz)
         ("revolute_joint_set_spring_damping_ratio", &nrevolute_joint_set_spring_damping_ratio)
         ("revolute_joint_enable_limit",    &nrevolute_joint_enable_limit)
         ("revolute_joint_set_limits",      &nrevolute_joint_set_limits)
         ("revolute_joint_enable_motor",    &nrevolute_joint_enable_motor)
         ("revolute_joint_set_motor_speed", &nrevolute_joint_set_motor_speed)
         ("revolute_joint_get_motor_speed", &nrevolute_joint_get_motor_speed)
         ("revolute_joint_set_max_motor_torque", &nrevolute_joint_set_max_motor_torque)
         ("revolute_joint_get_motor_torque", &nrevolute_joint_get_motor_torque);

        r("prismatic_joint_enable_spring",  &nprismatic_joint_enable_spring)
         ("prismatic_joint_set_spring_hertz", &nprismatic_joint_set_spring_hertz)
         ("prismatic_joint_set_spring_damping_ratio", &nprismatic_joint_set_spring_damping_ratio)
         ("prismatic_joint_enable_limit",   &nprismatic_joint_enable_limit)
         ("prismatic_joint_set_limits",     &nprismatic_joint_set_limits)
         ("prismatic_joint_enable_motor",   &nprismatic_joint_enable_motor)
         ("prismatic_joint_set_motor_speed", &nprismatic_joint_set_motor_speed)
         ("prismatic_joint_set_max_motor_force", &nprismatic_joint_set_max_motor_force)
         ("prismatic_joint_get_translation", &nprismatic_joint_get_translation)
         ("prismatic_joint_get_speed",      &nprismatic_joint_get_speed);

        r("weld_joint_set_linear_hertz",    &nweld_joint_set_linear_hertz)
         ("weld_joint_set_angular_hertz",   &nweld_joint_set_angular_hertz)
         ("weld_joint_set_linear_damping_ratio", &nweld_joint_set_linear_damping_ratio)
         ("weld_joint_set_angular_damping_ratio", &nweld_joint_set_angular_damping_ratio)
         ("weld_joint_set_reference_angle", &nweld_joint_set_reference_angle)
         ("weld_joint_get_reference_angle", &nweld_joint_get_reference_angle);

        r("motor_joint_set_linear_offset",  &nmotor_joint_set_linear_offset)
         ("motor_joint_set_angular_offset", &nmotor_joint_set_angular_offset)
         ("motor_joint_set_max_force",      &nmotor_joint_set_max_force)
         ("motor_joint_set_max_torque",     &nmotor_joint_set_max_torque)
         ("motor_joint_set_correction_factor", &nmotor_joint_set_correction_factor);

        r("mouse_joint_set_target",         &nmouse_joint_set_target)
         ("mouse_joint_set_spring_hertz",   &nmouse_joint_set_spring_hertz)
         ("mouse_joint_set_spring_damping_ratio", &nmouse_joint_set_spring_damping_ratio)
         ("mouse_joint_set_max_force",      &nmouse_joint_set_max_force);

        r("wheel_joint_enable_spring",      &nwheel_joint_enable_spring)
         ("wheel_joint_set_spring_hertz",   &nwheel_joint_set_spring_hertz)
         ("wheel_joint_set_spring_damping_ratio", &nwheel_joint_set_spring_damping_ratio)
         ("wheel_joint_enable_limit",       &nwheel_joint_enable_limit)
         ("wheel_joint_set_limits",         &nwheel_joint_set_limits)
         ("wheel_joint_enable_motor",       &nwheel_joint_enable_motor)
         ("wheel_joint_set_motor_speed",    &nwheel_joint_set_motor_speed)
         ("wheel_joint_set_max_motor_torque", &nwheel_joint_set_max_motor_torque)
         ("wheel_joint_get_motor_torque",   &nwheel_joint_get_motor_torque);
    }
}
