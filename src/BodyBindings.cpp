/*
 * Body operations: create / destroy from a body def, kinematics getters &
 * setters, force/impulse/torque application, mass data, sleep state, shape
 * enumeration, and an int64 user-data side-channel.
 *
 * The user-data slot is backed by a per-world unordered_map in WorldState
 * because b2Body_SetUserData(void*) truncates int64 on 32-bit hosts. The
 * map key is the packed body id (uint64).
 */

#include "PluginGlobals.hpp"
#include "BindingHelpers.hpp"
#include "WorldState.hpp"

#include "box2d/box2d.h"
#include "box2d/math_functions.h"

#include <vector>

namespace mtypeb2d
{
    namespace
    {
        constexpr const char* kEx = "Box2DError";
        using namespace detail;

        /* Find the WorldState for this body's world. May be null if the
         * body's world wasn't tracked (shouldn't happen in normal use). */
        WorldState* worldStateForBody(b2BodyId bodyId)
        {
            b2WorldId w = b2Body_GetWorld(bodyId);
            return findWorldState(worldToInt(w));
        }

        /* ---- lifecycle ---- */

        MTypeValue* nBodyCreate(void*, MTypeContext* ctx,
                                const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 2, "__native__b2d_body_create", kEx))
                return g_host->makeInt(ctx, 0);
            b2WorldId world = intToWorld(g_host->getInt(args[0]));
            if (!validateWorld(world, ctx, "__native__b2d_body_create", kEx))
                return g_host->makeInt(ctx, 0);
            auto* def = findDefOrRaise(g_bodyDefs, g_host->getInt(args[1]), ctx,
                                       "__native__b2d_body_create", kEx);
            if (!def) return g_host->makeInt(ctx, 0);
            b2BodyId id = b2CreateBody(world, def);
            return g_host->makeInt(ctx, bodyToInt(id));
        }

        MTypeValue* nBodyDestroy(void*, MTypeContext* ctx,
                                 const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_body_destroy", kEx))
                return g_host->makeVoid(ctx);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!b2Body_IsValid(id)) return g_host->makeVoid(ctx);
            /* Drop user-data side-map entry. */
            if (auto* ws = worldStateForBody(id)) {
                ws->bodyUserData.erase(b2StoreBodyId(id));
            }
            b2DestroyBody(id);
            return g_host->makeVoid(ctx);
        }

        /* ---- kinematics getters ---- */

        MTypeValue* nBodyGetPosition(void*, MTypeContext* ctx,
                                     const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_body_get_position", kEx))
                return emptyFloatArray(ctx);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_get_position", kEx))
                return emptyFloatArray(ctx);
            b2Vec2 p = b2Body_GetPosition(id);
            return makeFloatPair(ctx, p.x, p.y);
        }

        MTypeValue* nBodyGetRotation(void*, MTypeContext* ctx,
                                     const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_body_get_rotation", kEx))
                return g_host->makeFloat(ctx, 0.0);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_get_rotation", kEx))
                return g_host->makeFloat(ctx, 0.0);
            b2Rot r = b2Body_GetRotation(id);
            return g_host->makeFloat(ctx, b2Rot_GetAngle(r));
        }

        MTypeValue* nBodyGetLinearVelocity(void*, MTypeContext* ctx,
                                           const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_body_get_linear_velocity", kEx))
                return emptyFloatArray(ctx);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_get_linear_velocity", kEx))
                return emptyFloatArray(ctx);
            b2Vec2 v = b2Body_GetLinearVelocity(id);
            return makeFloatPair(ctx, v.x, v.y);
        }

        MTypeValue* nBodyGetAngularVelocity(void*, MTypeContext* ctx,
                                            const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_body_get_angular_velocity", kEx))
                return g_host->makeFloat(ctx, 0.0);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_get_angular_velocity", kEx))
                return g_host->makeFloat(ctx, 0.0);
            return g_host->makeFloat(ctx, b2Body_GetAngularVelocity(id));
        }

        MTypeValue* nBodyGetTransform(void*, MTypeContext* ctx,
                                      const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_body_get_transform", kEx))
                return emptyFloatArray(ctx);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_get_transform", kEx))
                return emptyFloatArray(ctx);
            b2Transform t = b2Body_GetTransform(id);
            return makeFloatQuad(ctx, t.p.x, t.p.y, t.q.c, t.q.s);
        }

        /* ---- kinematics setters ---- */

        MTypeValue* nBodySetTransform(void*, MTypeContext* ctx,
                                      const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 4, "__native__b2d_body_set_transform", kEx))
                return g_host->makeVoid(ctx);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_set_transform", kEx))
                return g_host->makeVoid(ctx);
            b2Body_SetTransform(id, getVec2(args[1], args[2]), b2MakeRot(getF(args[3])));
            return g_host->makeVoid(ctx);
        }

        MTypeValue* nBodySetLinearVelocity(void*, MTypeContext* ctx,
                                           const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 3, "__native__b2d_body_set_linear_velocity", kEx))
                return g_host->makeVoid(ctx);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_set_linear_velocity", kEx))
                return g_host->makeVoid(ctx);
            b2Body_SetLinearVelocity(id, getVec2(args[1], args[2]));
            return g_host->makeVoid(ctx);
        }

        MTypeValue* nBodySetAngularVelocity(void*, MTypeContext* ctx,
                                            const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 2, "__native__b2d_body_set_angular_velocity", kEx))
                return g_host->makeVoid(ctx);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_set_angular_velocity", kEx))
                return g_host->makeVoid(ctx);
            b2Body_SetAngularVelocity(id, getF(args[1]));
            return g_host->makeVoid(ctx);
        }

        MTypeValue* nBodySetType(void*, MTypeContext* ctx,
                                 const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 2, "__native__b2d_body_set_type", kEx))
                return g_host->makeVoid(ctx);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_set_type", kEx))
                return g_host->makeVoid(ctx);
            b2Body_SetType(id, static_cast<b2BodyType>(getI(args[1])));
            return g_host->makeVoid(ctx);
        }

        MTypeValue* nBodyGetType(void*, MTypeContext* ctx,
                                 const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_body_get_type", kEx))
                return g_host->makeInt(ctx, 0);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_get_type", kEx))
                return g_host->makeInt(ctx, 0);
            return g_host->makeInt(ctx, static_cast<int>(b2Body_GetType(id)));
        }

        /* ---- forces / impulses ---- */

        MTypeValue* nBodyApplyForce(void*, MTypeContext* ctx,
                                    const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 6, "__native__b2d_body_apply_force", kEx))
                return g_host->makeVoid(ctx);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_apply_force", kEx))
                return g_host->makeVoid(ctx);
            b2Body_ApplyForce(id,
                getVec2(args[1], args[2]),
                getVec2(args[3], args[4]),
                getB(args[5]));
            return g_host->makeVoid(ctx);
        }

        MTypeValue* nBodyApplyForceToCenter(void*, MTypeContext* ctx,
                                            const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 4, "__native__b2d_body_apply_force_to_center", kEx))
                return g_host->makeVoid(ctx);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_apply_force_to_center", kEx))
                return g_host->makeVoid(ctx);
            b2Body_ApplyForceToCenter(id, getVec2(args[1], args[2]), getB(args[3]));
            return g_host->makeVoid(ctx);
        }

        MTypeValue* nBodyApplyTorque(void*, MTypeContext* ctx,
                                     const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 3, "__native__b2d_body_apply_torque", kEx))
                return g_host->makeVoid(ctx);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_apply_torque", kEx))
                return g_host->makeVoid(ctx);
            b2Body_ApplyTorque(id, getF(args[1]), getB(args[2]));
            return g_host->makeVoid(ctx);
        }

        MTypeValue* nBodyApplyLinearImpulse(void*, MTypeContext* ctx,
                                            const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 6, "__native__b2d_body_apply_linear_impulse", kEx))
                return g_host->makeVoid(ctx);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_apply_linear_impulse", kEx))
                return g_host->makeVoid(ctx);
            b2Body_ApplyLinearImpulse(id,
                getVec2(args[1], args[2]),
                getVec2(args[3], args[4]),
                getB(args[5]));
            return g_host->makeVoid(ctx);
        }

        MTypeValue* nBodyApplyLinearImpulseToCenter(void*, MTypeContext* ctx,
                                                    const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 4, "__native__b2d_body_apply_linear_impulse_to_center", kEx))
                return g_host->makeVoid(ctx);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_apply_linear_impulse_to_center", kEx))
                return g_host->makeVoid(ctx);
            b2Body_ApplyLinearImpulseToCenter(id, getVec2(args[1], args[2]), getB(args[3]));
            return g_host->makeVoid(ctx);
        }

        MTypeValue* nBodyApplyAngularImpulse(void*, MTypeContext* ctx,
                                             const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 3, "__native__b2d_body_apply_angular_impulse", kEx))
                return g_host->makeVoid(ctx);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_apply_angular_impulse", kEx))
                return g_host->makeVoid(ctx);
            b2Body_ApplyAngularImpulse(id, getF(args[1]), getB(args[2]));
            return g_host->makeVoid(ctx);
        }

        /* ---- mass & damping ---- */

        MTypeValue* nBodyGetMass(void*, MTypeContext* ctx,
                                 const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_body_get_mass", kEx))
                return g_host->makeFloat(ctx, 0.0);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_get_mass", kEx))
                return g_host->makeFloat(ctx, 0.0);
            return g_host->makeFloat(ctx, b2Body_GetMass(id));
        }

        MTypeValue* nBodyGetRotationalInertia(void*, MTypeContext* ctx,
                                              const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_body_get_rotational_inertia", kEx))
                return g_host->makeFloat(ctx, 0.0);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_get_rotational_inertia", kEx))
                return g_host->makeFloat(ctx, 0.0);
            return g_host->makeFloat(ctx, b2Body_GetRotationalInertia(id));
        }

        MTypeValue* nBodyGetWorldCenterOfMass(void*, MTypeContext* ctx,
                                              const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_body_get_world_center", kEx))
                return emptyFloatArray(ctx);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_get_world_center", kEx))
                return emptyFloatArray(ctx);
            b2Vec2 c = b2Body_GetWorldCenterOfMass(id);
            return makeFloatPair(ctx, c.x, c.y);
        }

        MTypeValue* nBodyGetLocalCenterOfMass(void*, MTypeContext* ctx,
                                              const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_body_get_local_center", kEx))
                return emptyFloatArray(ctx);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_get_local_center", kEx))
                return emptyFloatArray(ctx);
            b2Vec2 c = b2Body_GetLocalCenterOfMass(id);
            return makeFloatPair(ctx, c.x, c.y);
        }

        MTypeValue* nBodySetMassData(void*, MTypeContext* ctx,
                                     const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 5, "__native__b2d_body_set_mass_data", kEx))
                return g_host->makeVoid(ctx);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_set_mass_data", kEx))
                return g_host->makeVoid(ctx);
            b2MassData md{};
            md.mass            = getF(args[1]);
            md.center.x        = getF(args[2]);
            md.center.y        = getF(args[3]);
            md.rotationalInertia = getF(args[4]);
            b2Body_SetMassData(id, md);
            return g_host->makeVoid(ctx);
        }

        MTypeValue* nBodyGetMassData(void*, MTypeContext* ctx,
                                     const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_body_get_mass_data", kEx))
                return emptyFloatArray(ctx);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_get_mass_data", kEx))
                return emptyFloatArray(ctx);
            b2MassData md = b2Body_GetMassData(id);
            return makeFloatQuad(ctx, md.mass, md.center.x, md.center.y, md.rotationalInertia);
        }

        MTypeValue* nBodyApplyMassFromShapes(void*, MTypeContext* ctx,
                                             const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_body_apply_mass_from_shapes", kEx))
                return g_host->makeVoid(ctx);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_apply_mass_from_shapes", kEx))
                return g_host->makeVoid(ctx);
            b2Body_ApplyMassFromShapes(id);
            return g_host->makeVoid(ctx);
        }

        #define BODY_SET1F(NAME, FN) \
        MTypeValue* nBody##NAME(void*, MTypeContext* ctx, \
                                const MTypeValue* const* args, int argc) \
        { \
            if (!requireArgs(ctx, argc, 2, "__native__b2d_body_" #NAME, kEx)) \
                return g_host->makeVoid(ctx); \
            b2BodyId id = intToBody(g_host->getInt(args[0])); \
            if (!validateBody(id, ctx, "__native__b2d_body_" #NAME, kEx)) \
                return g_host->makeVoid(ctx); \
            FN(id, getF(args[1])); \
            return g_host->makeVoid(ctx); \
        }

        BODY_SET1F(set_linear_damping,  b2Body_SetLinearDamping)
        BODY_SET1F(set_angular_damping, b2Body_SetAngularDamping)
        BODY_SET1F(set_gravity_scale,   b2Body_SetGravityScale)
        BODY_SET1F(set_sleep_threshold, b2Body_SetSleepThreshold)

        #undef BODY_SET1F

        #define BODY_SET1B(NAME, FN) \
        MTypeValue* nBody##NAME(void*, MTypeContext* ctx, \
                                const MTypeValue* const* args, int argc) \
        { \
            if (!requireArgs(ctx, argc, 2, "__native__b2d_body_" #NAME, kEx)) \
                return g_host->makeVoid(ctx); \
            b2BodyId id = intToBody(g_host->getInt(args[0])); \
            if (!validateBody(id, ctx, "__native__b2d_body_" #NAME, kEx)) \
                return g_host->makeVoid(ctx); \
            FN(id, getB(args[1])); \
            return g_host->makeVoid(ctx); \
        }

        BODY_SET1B(set_awake,           b2Body_SetAwake)
        BODY_SET1B(set_fixed_rotation,  b2Body_SetFixedRotation)
        BODY_SET1B(set_bullet,          b2Body_SetBullet)
        BODY_SET1B(enable_sleep,        b2Body_EnableSleep)
        BODY_SET1B(enable_contact_events, b2Body_EnableContactEvents)
        BODY_SET1B(enable_hit_events,   b2Body_EnableHitEvents)

        #undef BODY_SET1B

        /* ---- queries ---- */

        MTypeValue* nBodyIsAwake(void*, MTypeContext* ctx,
                                 const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_body_is_awake", kEx))
                return g_host->makeBool(ctx, 0);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_is_awake", kEx))
                return g_host->makeBool(ctx, 0);
            return g_host->makeBool(ctx, b2Body_IsAwake(id) ? 1 : 0);
        }

        MTypeValue* nBodyIsEnabled(void*, MTypeContext* ctx,
                                   const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_body_is_enabled", kEx))
                return g_host->makeBool(ctx, 0);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_is_enabled", kEx))
                return g_host->makeBool(ctx, 0);
            return g_host->makeBool(ctx, b2Body_IsEnabled(id) ? 1 : 0);
        }

        MTypeValue* nBodyEnable(void*, MTypeContext* ctx,
                                const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_body_enable", kEx))
                return g_host->makeVoid(ctx);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_enable", kEx))
                return g_host->makeVoid(ctx);
            b2Body_Enable(id);
            return g_host->makeVoid(ctx);
        }

        MTypeValue* nBodyDisable(void*, MTypeContext* ctx,
                                 const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_body_disable", kEx))
                return g_host->makeVoid(ctx);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_disable", kEx))
                return g_host->makeVoid(ctx);
            b2Body_Disable(id);
            return g_host->makeVoid(ctx);
        }

        /* ---- shapes / joints enumeration ---- */

        MTypeValue* nBodyGetShapeCount(void*, MTypeContext* ctx,
                                       const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_body_get_shape_count", kEx))
                return g_host->makeInt(ctx, 0);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_get_shape_count", kEx))
                return g_host->makeInt(ctx, 0);
            return g_host->makeInt(ctx, b2Body_GetShapeCount(id));
        }

        MTypeValue* nBodyGetShapes(void*, MTypeContext* ctx,
                                   const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_body_get_shapes", kEx))
                return emptyIntArray(ctx);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_get_shapes", kEx))
                return emptyIntArray(ctx);
            int n = b2Body_GetShapeCount(id);
            std::vector<b2ShapeId> ids(n);
            if (n > 0) b2Body_GetShapes(id, ids.data(), n);
            MTypeValue* out = g_host->makeArray(ctx, MT_TAG_INT, n);
            for (int i = 0; i < n; ++i)
                g_host->arraySet(out, i, g_host->makeInt(ctx, shapeToInt(ids[i])));
            return out;
        }

        MTypeValue* nBodyGetJointCount(void*, MTypeContext* ctx,
                                       const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_body_get_joint_count", kEx))
                return g_host->makeInt(ctx, 0);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_get_joint_count", kEx))
                return g_host->makeInt(ctx, 0);
            return g_host->makeInt(ctx, b2Body_GetJointCount(id));
        }

        MTypeValue* nBodyGetJoints(void*, MTypeContext* ctx,
                                   const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_body_get_joints", kEx))
                return emptyIntArray(ctx);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_get_joints", kEx))
                return emptyIntArray(ctx);
            int n = b2Body_GetJointCount(id);
            std::vector<b2JointId> ids(n);
            if (n > 0) b2Body_GetJoints(id, ids.data(), n);
            MTypeValue* out = g_host->makeArray(ctx, MT_TAG_INT, n);
            for (int i = 0; i < n; ++i)
                g_host->arraySet(out, i, g_host->makeInt(ctx, jointToInt(ids[i])));
            return out;
        }

        /* ---- user data int64 (side-map in WorldState) ---- */

        MTypeValue* nBodySetUserDataInt(void*, MTypeContext* ctx,
                                        const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 2, "__native__b2d_body_set_user_data_int", kEx))
                return g_host->makeVoid(ctx);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_set_user_data_int", kEx))
                return g_host->makeVoid(ctx);
            if (auto* ws = worldStateForBody(id)) {
                ws->bodyUserData[b2StoreBodyId(id)] = g_host->getInt(args[1]);
            }
            return g_host->makeVoid(ctx);
        }

        MTypeValue* nBodyGetUserDataInt(void*, MTypeContext* ctx,
                                        const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_body_get_user_data_int", kEx))
                return g_host->makeInt(ctx, 0);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_get_user_data_int", kEx))
                return g_host->makeInt(ctx, 0);
            if (auto* ws = worldStateForBody(id)) {
                auto it = ws->bodyUserData.find(b2StoreBodyId(id));
                if (it != ws->bodyUserData.end()) return g_host->makeInt(ctx, it->second);
            }
            return g_host->makeInt(ctx, 0);
        }

        MTypeValue* nBodyGetWorld(void*, MTypeContext* ctx,
                                  const MTypeValue* const* args, int argc)
        {
            if (!requireArgs(ctx, argc, 1, "__native__b2d_body_get_world", kEx))
                return g_host->makeInt(ctx, 0);
            b2BodyId id = intToBody(g_host->getInt(args[0]));
            if (!validateBody(id, ctx, "__native__b2d_body_get_world", kEx))
                return g_host->makeInt(ctx, 0);
            return g_host->makeInt(ctx, worldToInt(b2Body_GetWorld(id)));
        }
    }

    void registerBodyNatives(MTypeContext* ctx)
    {
        Registrar r{ctx, "__native__b2d_"};
        r("body_create",                       &nBodyCreate)
         ("body_destroy",                      &nBodyDestroy)
         ("body_get_position",                 &nBodyGetPosition)
         ("body_get_rotation",                 &nBodyGetRotation)
         ("body_get_transform",                &nBodyGetTransform)
         ("body_set_transform",                &nBodySetTransform)
         ("body_get_linear_velocity",          &nBodyGetLinearVelocity)
         ("body_set_linear_velocity",          &nBodySetLinearVelocity)
         ("body_get_angular_velocity",         &nBodyGetAngularVelocity)
         ("body_set_angular_velocity",         &nBodySetAngularVelocity)
         ("body_set_type",                     &nBodySetType)
         ("body_get_type",                     &nBodyGetType)
         ("body_apply_force",                  &nBodyApplyForce)
         ("body_apply_force_to_center",        &nBodyApplyForceToCenter)
         ("body_apply_torque",                 &nBodyApplyTorque)
         ("body_apply_linear_impulse",         &nBodyApplyLinearImpulse)
         ("body_apply_linear_impulse_to_center", &nBodyApplyLinearImpulseToCenter)
         ("body_apply_angular_impulse",        &nBodyApplyAngularImpulse)
         ("body_get_mass",                     &nBodyGetMass)
         ("body_get_rotational_inertia",       &nBodyGetRotationalInertia)
         ("body_get_world_center",             &nBodyGetWorldCenterOfMass)
         ("body_get_local_center",             &nBodyGetLocalCenterOfMass)
         ("body_set_mass_data",                &nBodySetMassData)
         ("body_get_mass_data",                &nBodyGetMassData)
         ("body_apply_mass_from_shapes",       &nBodyApplyMassFromShapes)
         ("body_set_linear_damping",           &nBodyset_linear_damping)
         ("body_set_angular_damping",          &nBodyset_angular_damping)
         ("body_set_gravity_scale",            &nBodyset_gravity_scale)
         ("body_set_sleep_threshold",          &nBodyset_sleep_threshold)
         ("body_set_awake",                    &nBodyset_awake)
         ("body_set_fixed_rotation",           &nBodyset_fixed_rotation)
         ("body_set_bullet",                   &nBodyset_bullet)
         ("body_enable_sleep",                 &nBodyenable_sleep)
         ("body_enable_contact_events",        &nBodyenable_contact_events)
         ("body_enable_hit_events",            &nBodyenable_hit_events)
         ("body_is_awake",                     &nBodyIsAwake)
         ("body_is_enabled",                   &nBodyIsEnabled)
         ("body_enable",                       &nBodyEnable)
         ("body_disable",                      &nBodyDisable)
         ("body_get_shape_count",              &nBodyGetShapeCount)
         ("body_get_shapes",                   &nBodyGetShapes)
         ("body_get_joint_count",              &nBodyGetJointCount)
         ("body_get_joints",                   &nBodyGetJoints)
         ("body_set_user_data_int",            &nBodySetUserDataInt)
         ("body_get_user_data_int",            &nBodyGetUserDataInt)
         ("body_get_world",                    &nBodyGetWorld);
    }
}
