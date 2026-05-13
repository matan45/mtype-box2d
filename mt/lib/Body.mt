// Body wrappers. A Body is a rigid object in the world; shapes attached
// to it via Shape.mt determine its collision geometry.

import * from "Box2D.mt";

// Builder for a body. Set fields, then pass to Bodies::create(world, def).
// Call .destroy() to release the plugin-side def heap object.
class BodyDef {
    public int handle;
    public constructor() { this.handle = __native__b2d_body_def_create(); }
    public function destroy(): void { __native__b2d_body_def_destroy(this.handle); }

    public function setType(int t): void              { __native__b2d_body_def_set_type(this.handle, t); }
    public function setPosition(float x, float y): void { __native__b2d_body_def_set_position(this.handle, x, y); }
    public function setAngle(float radians): void     { __native__b2d_body_def_set_angle(this.handle, radians); }
    public function setLinearVelocity(float vx, float vy): void {
        __native__b2d_body_def_set_linear_velocity(this.handle, vx, vy);
    }
    public function setAngularVelocity(float w): void  { __native__b2d_body_def_set_angular_velocity(this.handle, w); }
    public function setLinearDamping(float d): void    { __native__b2d_body_def_set_linear_damping(this.handle, d); }
    public function setAngularDamping(float d): void   { __native__b2d_body_def_set_angular_damping(this.handle, d); }
    public function setGravityScale(float s): void     { __native__b2d_body_def_set_gravity_scale(this.handle, s); }
    public function setSleepThreshold(float v): void   { __native__b2d_body_def_set_sleep_threshold(this.handle, v); }
    public function setEnableSleep(bool b): void       { __native__b2d_body_def_set_enable_sleep(this.handle, b); }
    public function setIsAwake(bool b): void           { __native__b2d_body_def_set_is_awake(this.handle, b); }
    public function setFixedRotation(bool b): void     { __native__b2d_body_def_set_fixed_rotation(this.handle, b); }
    public function setIsBullet(bool b): void          { __native__b2d_body_def_set_is_bullet(this.handle, b); }
    public function setIsEnabled(bool b): void         { __native__b2d_body_def_set_is_enabled(this.handle, b); }
}

class Body {
    public int handle;
    public constructor(int h) { this.handle = h; }
    public function destroy(): void { __native__b2d_body_destroy(this.handle); }

    // ---- kinematics ----
    public function position(): float[]       { return __native__b2d_body_get_position(this.handle); }
    public function rotation(): float          { return __native__b2d_body_get_rotation(this.handle); }
    // [px, py, cos, sin]
    public function transform(): float[]      { return __native__b2d_body_get_transform(this.handle); }
    public function setTransform(float x, float y, float radians): void {
        __native__b2d_body_set_transform(this.handle, x, y, radians);
    }
    public function linearVelocity(): float[] { return __native__b2d_body_get_linear_velocity(this.handle); }
    public function setLinearVelocity(float vx, float vy): void {
        __native__b2d_body_set_linear_velocity(this.handle, vx, vy);
    }
    public function angularVelocity(): float  { return __native__b2d_body_get_angular_velocity(this.handle); }
    public function setAngularVelocity(float w): void {
        __native__b2d_body_set_angular_velocity(this.handle, w);
    }
    public function getType(): int             { return __native__b2d_body_get_type(this.handle); }
    public function setType(int t): void       { __native__b2d_body_set_type(this.handle, t); }

    // ---- forces and impulses ----
    public function applyForce(float fx, float fy, float px, float py, bool wake): void {
        __native__b2d_body_apply_force(this.handle, fx, fy, px, py, wake);
    }
    public function applyForceToCenter(float fx, float fy, bool wake): void {
        __native__b2d_body_apply_force_to_center(this.handle, fx, fy, wake);
    }
    public function applyTorque(float t, bool wake): void {
        __native__b2d_body_apply_torque(this.handle, t, wake);
    }
    public function applyLinearImpulse(float ix, float iy, float px, float py, bool wake): void {
        __native__b2d_body_apply_linear_impulse(this.handle, ix, iy, px, py, wake);
    }
    public function applyLinearImpulseToCenter(float ix, float iy, bool wake): void {
        __native__b2d_body_apply_linear_impulse_to_center(this.handle, ix, iy, wake);
    }
    public function applyAngularImpulse(float i, bool wake): void {
        __native__b2d_body_apply_angular_impulse(this.handle, i, wake);
    }

    // ---- mass ----
    public function mass(): float                    { return __native__b2d_body_get_mass(this.handle); }
    public function rotationalInertia(): float        { return __native__b2d_body_get_rotational_inertia(this.handle); }
    public function worldCenter(): float[]          { return __native__b2d_body_get_world_center(this.handle); }
    public function localCenter(): float[]          { return __native__b2d_body_get_local_center(this.handle); }
    // [mass, comX, comY, rotInertia]
    public function massData(): float[]             { return __native__b2d_body_get_mass_data(this.handle); }
    public function setMassData(float mass, float comX, float comY, float rotInertia): void {
        __native__b2d_body_set_mass_data(this.handle, mass, comX, comY, rotInertia);
    }
    public function applyMassFromShapes(): void      { __native__b2d_body_apply_mass_from_shapes(this.handle); }

    // ---- damping & misc ----
    public function setLinearDamping(float d): void  { __native__b2d_body_set_linear_damping(this.handle, d); }
    public function setAngularDamping(float d): void { __native__b2d_body_set_angular_damping(this.handle, d); }
    public function setGravityScale(float s): void   { __native__b2d_body_set_gravity_scale(this.handle, s); }
    public function setSleepThreshold(float v): void { __native__b2d_body_set_sleep_threshold(this.handle, v); }

    // ---- state ----
    public function isAwake(): bool                   { return __native__b2d_body_is_awake(this.handle); }
    public function setAwake(bool b): void            { __native__b2d_body_set_awake(this.handle, b); }
    public function isEnabled(): bool                 { return __native__b2d_body_is_enabled(this.handle); }
    public function enable(): void                    { __native__b2d_body_enable(this.handle); }
    public function disable(): void                   { __native__b2d_body_disable(this.handle); }
    public function setFixedRotation(bool b): void    { __native__b2d_body_set_fixed_rotation(this.handle, b); }
    public function setBullet(bool b): void           { __native__b2d_body_set_bullet(this.handle, b); }
    public function enableSleep(bool b): void         { __native__b2d_body_enable_sleep(this.handle, b); }
    public function enableContactEvents(bool b): void { __native__b2d_body_enable_contact_events(this.handle, b); }
    public function enableHitEvents(bool b): void     { __native__b2d_body_enable_hit_events(this.handle, b); }

    // ---- shape/joint enumeration ----
    public function shapeCount(): int                 { return __native__b2d_body_get_shape_count(this.handle); }
    public function shapes(): int[]                   { return __native__b2d_body_get_shapes(this.handle); }
    public function jointCount(): int                 { return __native__b2d_body_get_joint_count(this.handle); }
    public function joints(): int[]                   { return __native__b2d_body_get_joints(this.handle); }

    // ---- user data int64 (side-channel) ----
    public function setUserDataInt(int v): void       { __native__b2d_body_set_user_data_int(this.handle, v); }
    public function userDataInt(): int                 { return __native__b2d_body_get_user_data_int(this.handle); }

    public function worldHandle(): int                { return __native__b2d_body_get_world(this.handle); }
}

class Bodies {
    public static function create(World w, BodyDef def): Body {
        return new Body(__native__b2d_body_create(w.handle, def.handle));
    }
}
