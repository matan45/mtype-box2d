/*
 * Plugin-side definition objects (b2WorldDef, b2BodyDef, b2ShapeDef,
 * b2ChainDef, per-joint defs) exposed to mType as int64 handles.
 *
 * mType code builds a def field-by-field through setters, then passes the
 * handle to the Create native (in WorldBindings/BodyBindings/etc.) which
 * reads the stored def and calls the matching b2Create*. The mType side is
 * responsible for calling `def.destroy()` to release the heap object.
 *
 * The pattern matches mtype-sfml's approach of avoiding ABI-level objGet on
 * user classes: every binding takes only primitives, so def construction is
 * a sequence of primitive setters on an opaque handle.
 */

#include "PluginGlobals.hpp"
#include "BindingHelpers.hpp"

#include "box2d/box2d.h"
#include "box2d/math_functions.h"
#include "box2d/types.h"

namespace mtypeb2d
{
    namespace
    {
        constexpr const char* kEx = "Box2DError";
        using namespace detail;

        /* =========================================================
         * b2WorldDef
         * ========================================================= */

        MTypeValue* nWorldDefCreate(void*, MTypeContext* ctx,
                                    const MTypeValue* const*, int argc)
        {
            (void)argc;
            auto* d = new b2WorldDef(b2DefaultWorldDef());
            return g_host->makeInt(ctx, g_worldDefs.insert(d));
        }
        MTypeValue* nWorldDefDestroy(void*, MTypeContext* ctx,
                                     const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_world_def_destroy", kEx))
                return g_host->makeVoid(ctx);
            delete g_worldDefs.erase(g_host->getInt(args[0]));
            return g_host->makeVoid(ctx);
        }

        #define WORLD_DEF_SET2(NAME, EXPR) \
        MTypeValue* nWorldDefSet_##NAME(void*, MTypeContext* ctx, \
                                        const MTypeValue* const* args, int argc) \
        { \
            if (!requireArgs(ctx, argc, 2, "__native__b2d_world_def_set_" #NAME, kEx)) \
                return g_host->makeVoid(ctx); \
            auto* d = findDefOrRaise(g_worldDefs, g_host->getInt(args[0]), ctx, \
                                     "__native__b2d_world_def_set_" #NAME, kEx); \
            if (!d) return g_host->makeVoid(ctx); \
            EXPR; \
            return g_host->makeVoid(ctx); \
        }

        WORLD_DEF_SET2(gravity_x,            d->gravity.x = getF(args[1]))
        WORLD_DEF_SET2(gravity_y,            d->gravity.y = getF(args[1]))
        WORLD_DEF_SET2(restitution_threshold, d->restitutionThreshold = getF(args[1]))
        WORLD_DEF_SET2(hit_event_threshold,  d->hitEventThreshold = getF(args[1]))
        WORLD_DEF_SET2(contact_hertz,        d->contactHertz = getF(args[1]))
        WORLD_DEF_SET2(contact_damping_ratio, d->contactDampingRatio = getF(args[1]))
        WORLD_DEF_SET2(max_contact_push_speed, d->maxContactPushSpeed = getF(args[1]))
        WORLD_DEF_SET2(joint_hertz,          d->jointHertz = getF(args[1]))
        WORLD_DEF_SET2(joint_damping_ratio,  d->jointDampingRatio = getF(args[1]))
        WORLD_DEF_SET2(maximum_linear_speed, d->maximumLinearSpeed = getF(args[1]))
        WORLD_DEF_SET2(enable_sleep,         d->enableSleep = getB(args[1]))
        WORLD_DEF_SET2(enable_continuous,    d->enableContinuous = getB(args[1]))
        WORLD_DEF_SET2(worker_count,         d->workerCount = getI(args[1]))

        #undef WORLD_DEF_SET2

        /* =========================================================
         * b2BodyDef
         * ========================================================= */

        MTypeValue* nBodyDefCreate(void*, MTypeContext* ctx,
                                   const MTypeValue* const*, int argc)
        {
            (void)argc;
            auto* d = new b2BodyDef(b2DefaultBodyDef());
            return g_host->makeInt(ctx, g_bodyDefs.insert(d));
        }
        MTypeValue* nBodyDefDestroy(void*, MTypeContext* ctx,
                                    const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_body_def_destroy", kEx))
                return g_host->makeVoid(ctx);
            delete g_bodyDefs.erase(g_host->getInt(args[0]));
            return g_host->makeVoid(ctx);
        }

        #define BODY_DEF_SET(EXPECTED, NAME, EXPR) \
        MTypeValue* nBodyDefSet_##NAME(void*, MTypeContext* ctx, \
                                       const MTypeValue* const* args, int argc) \
        { \
            if (!requireArgs(ctx, argc, EXPECTED, "__native__b2d_body_def_set_" #NAME, kEx)) \
                return g_host->makeVoid(ctx); \
            auto* d = findDefOrRaise(g_bodyDefs, g_host->getInt(args[0]), ctx, \
                                     "__native__b2d_body_def_set_" #NAME, kEx); \
            if (!d) return g_host->makeVoid(ctx); \
            EXPR; \
            return g_host->makeVoid(ctx); \
        }

        BODY_DEF_SET(2, type,           d->type = static_cast<b2BodyType>(getI(args[1])))
        BODY_DEF_SET(3, position,       d->position = getVec2(args[1], args[2]))
        BODY_DEF_SET(2, angle,          d->rotation = b2MakeRot(getF(args[1])))
        BODY_DEF_SET(3, linear_velocity, d->linearVelocity = getVec2(args[1], args[2]))
        BODY_DEF_SET(2, angular_velocity, d->angularVelocity = getF(args[1]))
        BODY_DEF_SET(2, linear_damping,  d->linearDamping = getF(args[1]))
        BODY_DEF_SET(2, angular_damping, d->angularDamping = getF(args[1]))
        BODY_DEF_SET(2, gravity_scale,   d->gravityScale = getF(args[1]))
        BODY_DEF_SET(2, sleep_threshold, d->sleepThreshold = getF(args[1]))
        BODY_DEF_SET(2, enable_sleep,    d->enableSleep = getB(args[1]))
        BODY_DEF_SET(2, is_awake,        d->isAwake = getB(args[1]))
        BODY_DEF_SET(2, fixed_rotation,  d->fixedRotation = getB(args[1]))
        BODY_DEF_SET(2, is_bullet,       d->isBullet = getB(args[1]))
        BODY_DEF_SET(2, is_enabled,      d->isEnabled = getB(args[1]))
        BODY_DEF_SET(2, allow_fast_rotation, d->allowFastRotation = getB(args[1]))

        #undef BODY_DEF_SET

        /* =========================================================
         * b2ShapeDef
         * ========================================================= */

        MTypeValue* nShapeDefCreate(void*, MTypeContext* ctx,
                                    const MTypeValue* const*, int argc)
        {
            (void)argc;
            auto* d = new b2ShapeDef(b2DefaultShapeDef());
            return g_host->makeInt(ctx, g_shapeDefs.insert(d));
        }
        MTypeValue* nShapeDefDestroy(void*, MTypeContext* ctx,
                                     const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_shape_def_destroy", kEx))
                return g_host->makeVoid(ctx);
            delete g_shapeDefs.erase(g_host->getInt(args[0]));
            return g_host->makeVoid(ctx);
        }

        #define SHAPE_DEF_SET(EXPECTED, NAME, EXPR) \
        MTypeValue* nShapeDefSet_##NAME(void*, MTypeContext* ctx, \
                                        const MTypeValue* const* args, int argc) \
        { \
            if (!requireArgs(ctx, argc, EXPECTED, "__native__b2d_shape_def_set_" #NAME, kEx)) \
                return g_host->makeVoid(ctx); \
            auto* d = findDefOrRaise(g_shapeDefs, g_host->getInt(args[0]), ctx, \
                                     "__native__b2d_shape_def_set_" #NAME, kEx); \
            if (!d) return g_host->makeVoid(ctx); \
            EXPR; \
            return g_host->makeVoid(ctx); \
        }

        SHAPE_DEF_SET(2, density,             d->density = getF(args[1]))
        SHAPE_DEF_SET(2, friction,            d->material.friction = getF(args[1]))
        SHAPE_DEF_SET(2, restitution,         d->material.restitution = getF(args[1]))
        SHAPE_DEF_SET(2, rolling_resistance,  d->material.rollingResistance = getF(args[1]))
        SHAPE_DEF_SET(2, tangent_speed,       d->material.tangentSpeed = getF(args[1]))
        SHAPE_DEF_SET(2, is_sensor,           d->isSensor = getB(args[1]))
        SHAPE_DEF_SET(4, filter,              (d->filter.categoryBits = getU64(args[1]),
                                               d->filter.maskBits     = getU64(args[2]),
                                               d->filter.groupIndex   = getI(args[3])))
        SHAPE_DEF_SET(2, enable_sensor_events,  d->enableSensorEvents = getB(args[1]))
        SHAPE_DEF_SET(2, enable_contact_events, d->enableContactEvents = getB(args[1]))
        SHAPE_DEF_SET(2, enable_hit_events,    d->enableHitEvents = getB(args[1]))
        SHAPE_DEF_SET(2, enable_pre_solve_events, d->enablePreSolveEvents = getB(args[1]))
        SHAPE_DEF_SET(2, update_body_mass,     d->updateBodyMass = getB(args[1]))

        #undef SHAPE_DEF_SET

        /* =========================================================
         * Generic per-joint-type def emitter.
         *
         * Each joint def gets:
         *  - <type>_joint_def_create()                            -> int64
         *  - <type>_joint_def_destroy(h)                          -> void
         *  - <type>_joint_def_set_bodies(h, bodyIdA, bodyIdB)     -> void
         *  - <type>_joint_def_set_collide_connected(h, bool)      -> void
         * plus type-specific setters defined inline.
         * ========================================================= */

        #define JOINT_DEF_LIFECYCLE_BASE(PREFIX, TYPE, DEFAULT_FN, REG) \
        MTypeValue* nJointDefCreate_##PREFIX(void*, MTypeContext* ctx, \
                                             const MTypeValue* const*, int) \
        { \
            auto* d = new TYPE(DEFAULT_FN()); \
            return g_host->makeInt(ctx, REG.insert(d)); \
        } \
        MTypeValue* nJointDefDestroy_##PREFIX(void*, MTypeContext* ctx, \
                                              const MTypeValue* const* args, int argc) \
        { \
            if (!requireArgs(ctx, argc, 1, "__native__b2d_" #PREFIX "_joint_def_destroy", kEx)) \
                return g_host->makeVoid(ctx); \
            delete REG.erase(g_host->getInt(args[0])); \
            return g_host->makeVoid(ctx); \
        } \
        MTypeValue* nJointDefBodies_##PREFIX(void*, MTypeContext* ctx, \
                                             const MTypeValue* const* args, int argc) \
        { \
            if (!requireArgs(ctx, argc, 3, "__native__b2d_" #PREFIX "_joint_def_set_bodies", kEx)) \
                return g_host->makeVoid(ctx); \
            auto* d = findDefOrRaise(REG, g_host->getInt(args[0]), ctx, \
                                     "__native__b2d_" #PREFIX "_joint_def_set_bodies", kEx); \
            if (!d) return g_host->makeVoid(ctx); \
            d->bodyIdA = intToBody(g_host->getInt(args[1])); \
            d->bodyIdB = intToBody(g_host->getInt(args[2])); \
            return g_host->makeVoid(ctx); \
        }

        /* Adds collideConnected on top of the base. Not used for filter
         * joints (b2FilterJointDef has no collideConnected field — filter
         * joints exist purely to suppress collision between two bodies). */
        #define JOINT_DEF_LIFECYCLE(PREFIX, TYPE, DEFAULT_FN, REG) \
        JOINT_DEF_LIFECYCLE_BASE(PREFIX, TYPE, DEFAULT_FN, REG) \
        MTypeValue* nJointDefCollide_##PREFIX(void*, MTypeContext* ctx, \
                                              const MTypeValue* const* args, int argc) \
        { \
            if (!requireArgs(ctx, argc, 2, "__native__b2d_" #PREFIX "_joint_def_set_collide_connected", kEx)) \
                return g_host->makeVoid(ctx); \
            auto* d = findDefOrRaise(REG, g_host->getInt(args[0]), ctx, \
                                     "__native__b2d_" #PREFIX "_joint_def_set_collide_connected", kEx); \
            if (!d) return g_host->makeVoid(ctx); \
            d->collideConnected = getB(args[1]); \
            return g_host->makeVoid(ctx); \
        }

        JOINT_DEF_LIFECYCLE(distance,  b2DistanceJointDef,  b2DefaultDistanceJointDef,  g_distanceJointDefs)
        JOINT_DEF_LIFECYCLE(revolute,  b2RevoluteJointDef,  b2DefaultRevoluteJointDef,  g_revoluteJointDefs)
        JOINT_DEF_LIFECYCLE(prismatic, b2PrismaticJointDef, b2DefaultPrismaticJointDef, g_prismaticJointDefs)
        JOINT_DEF_LIFECYCLE(weld,      b2WeldJointDef,      b2DefaultWeldJointDef,      g_weldJointDefs)
        JOINT_DEF_LIFECYCLE(motor,     b2MotorJointDef,     b2DefaultMotorJointDef,     g_motorJointDefs)
        JOINT_DEF_LIFECYCLE(mouse,     b2MouseJointDef,     b2DefaultMouseJointDef,     g_mouseJointDefs)
        JOINT_DEF_LIFECYCLE(wheel,     b2WheelJointDef,     b2DefaultWheelJointDef,     g_wheelJointDefs)
        JOINT_DEF_LIFECYCLE_BASE(filter, b2FilterJointDef,  b2DefaultFilterJointDef,    g_filterJointDefs)

        #undef JOINT_DEF_LIFECYCLE
        #undef JOINT_DEF_LIFECYCLE_BASE

        /* Per-joint-type field setters. argc=3 -> (handle, x, y); argc=2 -> (handle, value). */

        #define JOINT_DEF_FIELD(PREFIX, REG, NAME, EXPECTED, EXPR) \
        MTypeValue* nJointDefField_##PREFIX##_##NAME(void*, MTypeContext* ctx, \
                                                     const MTypeValue* const* args, int argc) \
        { \
            if (!requireArgs(ctx, argc, EXPECTED, "__native__b2d_" #PREFIX "_joint_def_set_" #NAME, kEx)) \
                return g_host->makeVoid(ctx); \
            auto* d = findDefOrRaise(REG, g_host->getInt(args[0]), ctx, \
                                     "__native__b2d_" #PREFIX "_joint_def_set_" #NAME, kEx); \
            if (!d) return g_host->makeVoid(ctx); \
            EXPR; \
            return g_host->makeVoid(ctx); \
        }

        /* ---- distance ---- */
        JOINT_DEF_FIELD(distance, g_distanceJointDefs, local_anchor_a, 3, d->localAnchorA = getVec2(args[1], args[2]))
        JOINT_DEF_FIELD(distance, g_distanceJointDefs, local_anchor_b, 3, d->localAnchorB = getVec2(args[1], args[2]))
        JOINT_DEF_FIELD(distance, g_distanceJointDefs, length,         2, d->length = getF(args[1]))
        JOINT_DEF_FIELD(distance, g_distanceJointDefs, enable_spring,  2, d->enableSpring = getB(args[1]))
        JOINT_DEF_FIELD(distance, g_distanceJointDefs, hertz,          2, d->hertz = getF(args[1]))
        JOINT_DEF_FIELD(distance, g_distanceJointDefs, damping_ratio,  2, d->dampingRatio = getF(args[1]))
        JOINT_DEF_FIELD(distance, g_distanceJointDefs, enable_limit,   2, d->enableLimit = getB(args[1]))
        JOINT_DEF_FIELD(distance, g_distanceJointDefs, min_length,     2, d->minLength = getF(args[1]))
        JOINT_DEF_FIELD(distance, g_distanceJointDefs, max_length,     2, d->maxLength = getF(args[1]))
        JOINT_DEF_FIELD(distance, g_distanceJointDefs, enable_motor,   2, d->enableMotor = getB(args[1]))
        JOINT_DEF_FIELD(distance, g_distanceJointDefs, max_motor_force, 2, d->maxMotorForce = getF(args[1]))
        JOINT_DEF_FIELD(distance, g_distanceJointDefs, motor_speed,    2, d->motorSpeed = getF(args[1]))

        /* ---- revolute ---- */
        JOINT_DEF_FIELD(revolute, g_revoluteJointDefs, local_anchor_a, 3, d->localAnchorA = getVec2(args[1], args[2]))
        JOINT_DEF_FIELD(revolute, g_revoluteJointDefs, local_anchor_b, 3, d->localAnchorB = getVec2(args[1], args[2]))
        JOINT_DEF_FIELD(revolute, g_revoluteJointDefs, reference_angle, 2, d->referenceAngle = getF(args[1]))
        JOINT_DEF_FIELD(revolute, g_revoluteJointDefs, enable_spring,   2, d->enableSpring = getB(args[1]))
        JOINT_DEF_FIELD(revolute, g_revoluteJointDefs, hertz,           2, d->hertz = getF(args[1]))
        JOINT_DEF_FIELD(revolute, g_revoluteJointDefs, damping_ratio,   2, d->dampingRatio = getF(args[1]))
        JOINT_DEF_FIELD(revolute, g_revoluteJointDefs, enable_limit,    2, d->enableLimit = getB(args[1]))
        JOINT_DEF_FIELD(revolute, g_revoluteJointDefs, lower_angle,     2, d->lowerAngle = getF(args[1]))
        JOINT_DEF_FIELD(revolute, g_revoluteJointDefs, upper_angle,     2, d->upperAngle = getF(args[1]))
        JOINT_DEF_FIELD(revolute, g_revoluteJointDefs, enable_motor,    2, d->enableMotor = getB(args[1]))
        JOINT_DEF_FIELD(revolute, g_revoluteJointDefs, max_motor_torque, 2, d->maxMotorTorque = getF(args[1]))
        JOINT_DEF_FIELD(revolute, g_revoluteJointDefs, motor_speed,     2, d->motorSpeed = getF(args[1]))
        JOINT_DEF_FIELD(revolute, g_revoluteJointDefs, draw_size,       2, d->drawSize = getF(args[1]))

        /* ---- prismatic ---- */
        JOINT_DEF_FIELD(prismatic, g_prismaticJointDefs, local_anchor_a, 3, d->localAnchorA = getVec2(args[1], args[2]))
        JOINT_DEF_FIELD(prismatic, g_prismaticJointDefs, local_anchor_b, 3, d->localAnchorB = getVec2(args[1], args[2]))
        JOINT_DEF_FIELD(prismatic, g_prismaticJointDefs, local_axis_a,   3, d->localAxisA   = getVec2(args[1], args[2]))
        JOINT_DEF_FIELD(prismatic, g_prismaticJointDefs, reference_angle, 2, d->referenceAngle = getF(args[1]))
        JOINT_DEF_FIELD(prismatic, g_prismaticJointDefs, enable_spring,  2, d->enableSpring = getB(args[1]))
        JOINT_DEF_FIELD(prismatic, g_prismaticJointDefs, hertz,          2, d->hertz = getF(args[1]))
        JOINT_DEF_FIELD(prismatic, g_prismaticJointDefs, damping_ratio,  2, d->dampingRatio = getF(args[1]))
        JOINT_DEF_FIELD(prismatic, g_prismaticJointDefs, enable_limit,   2, d->enableLimit = getB(args[1]))
        JOINT_DEF_FIELD(prismatic, g_prismaticJointDefs, lower_translation, 2, d->lowerTranslation = getF(args[1]))
        JOINT_DEF_FIELD(prismatic, g_prismaticJointDefs, upper_translation, 2, d->upperTranslation = getF(args[1]))
        JOINT_DEF_FIELD(prismatic, g_prismaticJointDefs, enable_motor,   2, d->enableMotor = getB(args[1]))
        JOINT_DEF_FIELD(prismatic, g_prismaticJointDefs, max_motor_force, 2, d->maxMotorForce = getF(args[1]))
        JOINT_DEF_FIELD(prismatic, g_prismaticJointDefs, motor_speed,    2, d->motorSpeed = getF(args[1]))

        /* ---- weld ---- */
        JOINT_DEF_FIELD(weld, g_weldJointDefs, local_anchor_a,   3, d->localAnchorA = getVec2(args[1], args[2]))
        JOINT_DEF_FIELD(weld, g_weldJointDefs, local_anchor_b,   3, d->localAnchorB = getVec2(args[1], args[2]))
        JOINT_DEF_FIELD(weld, g_weldJointDefs, reference_angle,  2, d->referenceAngle = getF(args[1]))
        JOINT_DEF_FIELD(weld, g_weldJointDefs, linear_hertz,     2, d->linearHertz = getF(args[1]))
        JOINT_DEF_FIELD(weld, g_weldJointDefs, angular_hertz,    2, d->angularHertz = getF(args[1]))
        JOINT_DEF_FIELD(weld, g_weldJointDefs, linear_damping_ratio, 2, d->linearDampingRatio = getF(args[1]))
        JOINT_DEF_FIELD(weld, g_weldJointDefs, angular_damping_ratio, 2, d->angularDampingRatio = getF(args[1]))

        /* ---- motor ---- */
        JOINT_DEF_FIELD(motor, g_motorJointDefs, linear_offset,    3, d->linearOffset = getVec2(args[1], args[2]))
        JOINT_DEF_FIELD(motor, g_motorJointDefs, angular_offset,   2, d->angularOffset = getF(args[1]))
        JOINT_DEF_FIELD(motor, g_motorJointDefs, max_force,        2, d->maxForce = getF(args[1]))
        JOINT_DEF_FIELD(motor, g_motorJointDefs, max_torque,       2, d->maxTorque = getF(args[1]))
        JOINT_DEF_FIELD(motor, g_motorJointDefs, correction_factor, 2, d->correctionFactor = getF(args[1]))

        /* ---- mouse ---- */
        JOINT_DEF_FIELD(mouse, g_mouseJointDefs, target,         3, d->target = getVec2(args[1], args[2]))
        JOINT_DEF_FIELD(mouse, g_mouseJointDefs, hertz,          2, d->hertz = getF(args[1]))
        JOINT_DEF_FIELD(mouse, g_mouseJointDefs, damping_ratio,  2, d->dampingRatio = getF(args[1]))
        JOINT_DEF_FIELD(mouse, g_mouseJointDefs, max_force,      2, d->maxForce = getF(args[1]))

        /* ---- wheel ---- */
        JOINT_DEF_FIELD(wheel, g_wheelJointDefs, local_anchor_a,   3, d->localAnchorA = getVec2(args[1], args[2]))
        JOINT_DEF_FIELD(wheel, g_wheelJointDefs, local_anchor_b,   3, d->localAnchorB = getVec2(args[1], args[2]))
        JOINT_DEF_FIELD(wheel, g_wheelJointDefs, local_axis_a,     3, d->localAxisA   = getVec2(args[1], args[2]))
        JOINT_DEF_FIELD(wheel, g_wheelJointDefs, enable_spring,    2, d->enableSpring = getB(args[1]))
        JOINT_DEF_FIELD(wheel, g_wheelJointDefs, hertz,            2, d->hertz = getF(args[1]))
        JOINT_DEF_FIELD(wheel, g_wheelJointDefs, damping_ratio,    2, d->dampingRatio = getF(args[1]))
        JOINT_DEF_FIELD(wheel, g_wheelJointDefs, enable_limit,     2, d->enableLimit = getB(args[1]))
        JOINT_DEF_FIELD(wheel, g_wheelJointDefs, lower_translation, 2, d->lowerTranslation = getF(args[1]))
        JOINT_DEF_FIELD(wheel, g_wheelJointDefs, upper_translation, 2, d->upperTranslation = getF(args[1]))
        JOINT_DEF_FIELD(wheel, g_wheelJointDefs, enable_motor,     2, d->enableMotor = getB(args[1]))
        JOINT_DEF_FIELD(wheel, g_wheelJointDefs, max_motor_torque, 2, d->maxMotorTorque = getF(args[1]))
        JOINT_DEF_FIELD(wheel, g_wheelJointDefs, motor_speed,      2, d->motorSpeed = getF(args[1]))

        #undef JOINT_DEF_FIELD
    }

    void registerDefNatives(MTypeContext* ctx)
    {
        Registrar r{ctx, "__native__b2d_"};

        /* World def */
        r("world_def_create",                  &nWorldDefCreate)
         ("world_def_destroy",                 &nWorldDefDestroy)
         ("world_def_set_gravity_x",           &nWorldDefSet_gravity_x)
         ("world_def_set_gravity_y",           &nWorldDefSet_gravity_y)
         ("world_def_set_restitution_threshold", &nWorldDefSet_restitution_threshold)
         ("world_def_set_hit_event_threshold", &nWorldDefSet_hit_event_threshold)
         ("world_def_set_contact_hertz",       &nWorldDefSet_contact_hertz)
         ("world_def_set_contact_damping_ratio", &nWorldDefSet_contact_damping_ratio)
         ("world_def_set_max_contact_push_speed", &nWorldDefSet_max_contact_push_speed)
         ("world_def_set_joint_hertz",         &nWorldDefSet_joint_hertz)
         ("world_def_set_joint_damping_ratio", &nWorldDefSet_joint_damping_ratio)
         ("world_def_set_maximum_linear_speed", &nWorldDefSet_maximum_linear_speed)
         ("world_def_set_enable_sleep",        &nWorldDefSet_enable_sleep)
         ("world_def_set_enable_continuous",   &nWorldDefSet_enable_continuous)
         ("world_def_set_worker_count",        &nWorldDefSet_worker_count);

        /* Body def */
        r("body_def_create",                   &nBodyDefCreate)
         ("body_def_destroy",                  &nBodyDefDestroy)
         ("body_def_set_type",                 &nBodyDefSet_type)
         ("body_def_set_position",             &nBodyDefSet_position)
         ("body_def_set_angle",                &nBodyDefSet_angle)
         ("body_def_set_linear_velocity",      &nBodyDefSet_linear_velocity)
         ("body_def_set_angular_velocity",     &nBodyDefSet_angular_velocity)
         ("body_def_set_linear_damping",       &nBodyDefSet_linear_damping)
         ("body_def_set_angular_damping",      &nBodyDefSet_angular_damping)
         ("body_def_set_gravity_scale",        &nBodyDefSet_gravity_scale)
         ("body_def_set_sleep_threshold",      &nBodyDefSet_sleep_threshold)
         ("body_def_set_enable_sleep",         &nBodyDefSet_enable_sleep)
         ("body_def_set_is_awake",             &nBodyDefSet_is_awake)
         ("body_def_set_fixed_rotation",       &nBodyDefSet_fixed_rotation)
         ("body_def_set_is_bullet",            &nBodyDefSet_is_bullet)
         ("body_def_set_is_enabled",           &nBodyDefSet_is_enabled)
         ("body_def_set_allow_fast_rotation",  &nBodyDefSet_allow_fast_rotation);

        /* Shape def */
        r("shape_def_create",                  &nShapeDefCreate)
         ("shape_def_destroy",                 &nShapeDefDestroy)
         ("shape_def_set_density",             &nShapeDefSet_density)
         ("shape_def_set_friction",            &nShapeDefSet_friction)
         ("shape_def_set_restitution",         &nShapeDefSet_restitution)
         ("shape_def_set_rolling_resistance",  &nShapeDefSet_rolling_resistance)
         ("shape_def_set_tangent_speed",       &nShapeDefSet_tangent_speed)
         ("shape_def_set_is_sensor",           &nShapeDefSet_is_sensor)
         ("shape_def_set_filter",              &nShapeDefSet_filter)
         ("shape_def_set_enable_sensor_events", &nShapeDefSet_enable_sensor_events)
         ("shape_def_set_enable_contact_events", &nShapeDefSet_enable_contact_events)
         ("shape_def_set_enable_hit_events",   &nShapeDefSet_enable_hit_events)
         ("shape_def_set_enable_pre_solve_events", &nShapeDefSet_enable_pre_solve_events)
         ("shape_def_set_update_body_mass",    &nShapeDefSet_update_body_mass);

        /* Joint defs — lifecycle + bodies + collideConnected + per-type fields. */

        #define REG_JOINT_DEF_BASE(PREFIX) \
            r(#PREFIX "_joint_def_create",                  &nJointDefCreate_##PREFIX) \
             (#PREFIX "_joint_def_destroy",                 &nJointDefDestroy_##PREFIX) \
             (#PREFIX "_joint_def_set_bodies",              &nJointDefBodies_##PREFIX)

        #define REG_JOINT_DEF_FULL(PREFIX) \
            REG_JOINT_DEF_BASE(PREFIX) \
             (#PREFIX "_joint_def_set_collide_connected",   &nJointDefCollide_##PREFIX)

        REG_JOINT_DEF_FULL(distance);
        REG_JOINT_DEF_FULL(revolute);
        REG_JOINT_DEF_FULL(prismatic);
        REG_JOINT_DEF_FULL(weld);
        REG_JOINT_DEF_FULL(motor);
        REG_JOINT_DEF_FULL(mouse);
        REG_JOINT_DEF_FULL(wheel);
        REG_JOINT_DEF_BASE(filter);
        #undef REG_JOINT_DEF_BASE
        #undef REG_JOINT_DEF_FULL

        #define REG_JOINT_FIELD(PREFIX, NAME) \
            r(#PREFIX "_joint_def_set_" #NAME, &nJointDefField_##PREFIX##_##NAME)

        REG_JOINT_FIELD(distance, local_anchor_a);
        REG_JOINT_FIELD(distance, local_anchor_b);
        REG_JOINT_FIELD(distance, length);
        REG_JOINT_FIELD(distance, enable_spring);
        REG_JOINT_FIELD(distance, hertz);
        REG_JOINT_FIELD(distance, damping_ratio);
        REG_JOINT_FIELD(distance, enable_limit);
        REG_JOINT_FIELD(distance, min_length);
        REG_JOINT_FIELD(distance, max_length);
        REG_JOINT_FIELD(distance, enable_motor);
        REG_JOINT_FIELD(distance, max_motor_force);
        REG_JOINT_FIELD(distance, motor_speed);

        REG_JOINT_FIELD(revolute, local_anchor_a);
        REG_JOINT_FIELD(revolute, local_anchor_b);
        REG_JOINT_FIELD(revolute, reference_angle);
        REG_JOINT_FIELD(revolute, enable_spring);
        REG_JOINT_FIELD(revolute, hertz);
        REG_JOINT_FIELD(revolute, damping_ratio);
        REG_JOINT_FIELD(revolute, enable_limit);
        REG_JOINT_FIELD(revolute, lower_angle);
        REG_JOINT_FIELD(revolute, upper_angle);
        REG_JOINT_FIELD(revolute, enable_motor);
        REG_JOINT_FIELD(revolute, max_motor_torque);
        REG_JOINT_FIELD(revolute, motor_speed);
        REG_JOINT_FIELD(revolute, draw_size);

        REG_JOINT_FIELD(prismatic, local_anchor_a);
        REG_JOINT_FIELD(prismatic, local_anchor_b);
        REG_JOINT_FIELD(prismatic, local_axis_a);
        REG_JOINT_FIELD(prismatic, reference_angle);
        REG_JOINT_FIELD(prismatic, enable_spring);
        REG_JOINT_FIELD(prismatic, hertz);
        REG_JOINT_FIELD(prismatic, damping_ratio);
        REG_JOINT_FIELD(prismatic, enable_limit);
        REG_JOINT_FIELD(prismatic, lower_translation);
        REG_JOINT_FIELD(prismatic, upper_translation);
        REG_JOINT_FIELD(prismatic, enable_motor);
        REG_JOINT_FIELD(prismatic, max_motor_force);
        REG_JOINT_FIELD(prismatic, motor_speed);

        REG_JOINT_FIELD(weld, local_anchor_a);
        REG_JOINT_FIELD(weld, local_anchor_b);
        REG_JOINT_FIELD(weld, reference_angle);
        REG_JOINT_FIELD(weld, linear_hertz);
        REG_JOINT_FIELD(weld, angular_hertz);
        REG_JOINT_FIELD(weld, linear_damping_ratio);
        REG_JOINT_FIELD(weld, angular_damping_ratio);

        REG_JOINT_FIELD(motor, linear_offset);
        REG_JOINT_FIELD(motor, angular_offset);
        REG_JOINT_FIELD(motor, max_force);
        REG_JOINT_FIELD(motor, max_torque);
        REG_JOINT_FIELD(motor, correction_factor);

        REG_JOINT_FIELD(mouse, target);
        REG_JOINT_FIELD(mouse, hertz);
        REG_JOINT_FIELD(mouse, damping_ratio);
        REG_JOINT_FIELD(mouse, max_force);

        REG_JOINT_FIELD(wheel, local_anchor_a);
        REG_JOINT_FIELD(wheel, local_anchor_b);
        REG_JOINT_FIELD(wheel, local_axis_a);
        REG_JOINT_FIELD(wheel, enable_spring);
        REG_JOINT_FIELD(wheel, hertz);
        REG_JOINT_FIELD(wheel, damping_ratio);
        REG_JOINT_FIELD(wheel, enable_limit);
        REG_JOINT_FIELD(wheel, lower_translation);
        REG_JOINT_FIELD(wheel, upper_translation);
        REG_JOINT_FIELD(wheel, enable_motor);
        REG_JOINT_FIELD(wheel, max_motor_torque);
        REG_JOINT_FIELD(wheel, motor_speed);

        #undef REG_JOINT_FIELD
    }
}
