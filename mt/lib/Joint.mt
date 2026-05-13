// Joint wrappers. Each joint type has its own def class; build, configure,
// pass to Joints::createXxx, then destroy the def.

import * from "Box2D.mt";
import * from "Body.mt";

// ---- Distance joint -------------------------------------------------------

class DistanceJointDef {
    public int handle;
    public constructor() { this.handle = __native__b2d_distance_joint_def_create(); }
    public function destroy(): void { __native__b2d_distance_joint_def_destroy(this.handle); }
    public function setBodies(Body a, Body b): void {
        __native__b2d_distance_joint_def_set_bodies(this.handle, a.handle, b.handle);
    }
    public function setLocalAnchorA(float x, float y): void {
        __native__b2d_distance_joint_def_set_local_anchor_a(this.handle, x, y);
    }
    public function setLocalAnchorB(float x, float y): void {
        __native__b2d_distance_joint_def_set_local_anchor_b(this.handle, x, y);
    }
    public function setLength(float l): void          { __native__b2d_distance_joint_def_set_length(this.handle, l); }
    public function setEnableSpring(bool b): void     { __native__b2d_distance_joint_def_set_enable_spring(this.handle, b); }
    public function setHertz(float h): void           { __native__b2d_distance_joint_def_set_hertz(this.handle, h); }
    public function setDampingRatio(float d): void    { __native__b2d_distance_joint_def_set_damping_ratio(this.handle, d); }
    public function setEnableLimit(bool b): void      { __native__b2d_distance_joint_def_set_enable_limit(this.handle, b); }
    public function setMinLength(float v): void       { __native__b2d_distance_joint_def_set_min_length(this.handle, v); }
    public function setMaxLength(float v): void       { __native__b2d_distance_joint_def_set_max_length(this.handle, v); }
    public function setEnableMotor(bool b): void      { __native__b2d_distance_joint_def_set_enable_motor(this.handle, b); }
    public function setMaxMotorForce(float v): void   { __native__b2d_distance_joint_def_set_max_motor_force(this.handle, v); }
    public function setMotorSpeed(float v): void      { __native__b2d_distance_joint_def_set_motor_speed(this.handle, v); }
    public function setCollideConnected(bool b): void { __native__b2d_distance_joint_def_set_collide_connected(this.handle, b); }
}

// ---- Revolute joint -------------------------------------------------------

class RevoluteJointDef {
    public int handle;
    public constructor() { this.handle = __native__b2d_revolute_joint_def_create(); }
    public function destroy(): void { __native__b2d_revolute_joint_def_destroy(this.handle); }
    public function setBodies(Body a, Body b): void {
        __native__b2d_revolute_joint_def_set_bodies(this.handle, a.handle, b.handle);
    }
    public function setLocalAnchorA(float x, float y): void { __native__b2d_revolute_joint_def_set_local_anchor_a(this.handle, x, y); }
    public function setLocalAnchorB(float x, float y): void { __native__b2d_revolute_joint_def_set_local_anchor_b(this.handle, x, y); }
    public function setReferenceAngle(float r): void      { __native__b2d_revolute_joint_def_set_reference_angle(this.handle, r); }
    public function setEnableSpring(bool b): void         { __native__b2d_revolute_joint_def_set_enable_spring(this.handle, b); }
    public function setHertz(float h): void               { __native__b2d_revolute_joint_def_set_hertz(this.handle, h); }
    public function setDampingRatio(float d): void        { __native__b2d_revolute_joint_def_set_damping_ratio(this.handle, d); }
    public function setEnableLimit(bool b): void          { __native__b2d_revolute_joint_def_set_enable_limit(this.handle, b); }
    public function setLowerAngle(float r): void          { __native__b2d_revolute_joint_def_set_lower_angle(this.handle, r); }
    public function setUpperAngle(float r): void          { __native__b2d_revolute_joint_def_set_upper_angle(this.handle, r); }
    public function setEnableMotor(bool b): void          { __native__b2d_revolute_joint_def_set_enable_motor(this.handle, b); }
    public function setMaxMotorTorque(float v): void      { __native__b2d_revolute_joint_def_set_max_motor_torque(this.handle, v); }
    public function setMotorSpeed(float v): void          { __native__b2d_revolute_joint_def_set_motor_speed(this.handle, v); }
    public function setDrawSize(float v): void            { __native__b2d_revolute_joint_def_set_draw_size(this.handle, v); }
    public function setCollideConnected(bool b): void     { __native__b2d_revolute_joint_def_set_collide_connected(this.handle, b); }
}

// ---- Prismatic joint ------------------------------------------------------

class PrismaticJointDef {
    public int handle;
    public constructor() { this.handle = __native__b2d_prismatic_joint_def_create(); }
    public function destroy(): void { __native__b2d_prismatic_joint_def_destroy(this.handle); }
    public function setBodies(Body a, Body b): void {
        __native__b2d_prismatic_joint_def_set_bodies(this.handle, a.handle, b.handle);
    }
    public function setLocalAnchorA(float x, float y): void { __native__b2d_prismatic_joint_def_set_local_anchor_a(this.handle, x, y); }
    public function setLocalAnchorB(float x, float y): void { __native__b2d_prismatic_joint_def_set_local_anchor_b(this.handle, x, y); }
    public function setLocalAxisA(float x, float y): void   { __native__b2d_prismatic_joint_def_set_local_axis_a(this.handle, x, y); }
    public function setReferenceAngle(float r): void       { __native__b2d_prismatic_joint_def_set_reference_angle(this.handle, r); }
    public function setEnableSpring(bool b): void          { __native__b2d_prismatic_joint_def_set_enable_spring(this.handle, b); }
    public function setHertz(float h): void                { __native__b2d_prismatic_joint_def_set_hertz(this.handle, h); }
    public function setDampingRatio(float d): void         { __native__b2d_prismatic_joint_def_set_damping_ratio(this.handle, d); }
    public function setEnableLimit(bool b): void           { __native__b2d_prismatic_joint_def_set_enable_limit(this.handle, b); }
    public function setLowerTranslation(float v): void     { __native__b2d_prismatic_joint_def_set_lower_translation(this.handle, v); }
    public function setUpperTranslation(float v): void     { __native__b2d_prismatic_joint_def_set_upper_translation(this.handle, v); }
    public function setEnableMotor(bool b): void           { __native__b2d_prismatic_joint_def_set_enable_motor(this.handle, b); }
    public function setMaxMotorForce(float v): void        { __native__b2d_prismatic_joint_def_set_max_motor_force(this.handle, v); }
    public function setMotorSpeed(float v): void           { __native__b2d_prismatic_joint_def_set_motor_speed(this.handle, v); }
    public function setCollideConnected(bool b): void      { __native__b2d_prismatic_joint_def_set_collide_connected(this.handle, b); }
}

// ---- Weld joint -----------------------------------------------------------

class WeldJointDef {
    public int handle;
    public constructor() { this.handle = __native__b2d_weld_joint_def_create(); }
    public function destroy(): void { __native__b2d_weld_joint_def_destroy(this.handle); }
    public function setBodies(Body a, Body b): void {
        __native__b2d_weld_joint_def_set_bodies(this.handle, a.handle, b.handle);
    }
    public function setLocalAnchorA(float x, float y): void { __native__b2d_weld_joint_def_set_local_anchor_a(this.handle, x, y); }
    public function setLocalAnchorB(float x, float y): void { __native__b2d_weld_joint_def_set_local_anchor_b(this.handle, x, y); }
    public function setReferenceAngle(float r): void       { __native__b2d_weld_joint_def_set_reference_angle(this.handle, r); }
    public function setLinearHertz(float h): void          { __native__b2d_weld_joint_def_set_linear_hertz(this.handle, h); }
    public function setAngularHertz(float h): void         { __native__b2d_weld_joint_def_set_angular_hertz(this.handle, h); }
    public function setLinearDampingRatio(float d): void   { __native__b2d_weld_joint_def_set_linear_damping_ratio(this.handle, d); }
    public function setAngularDampingRatio(float d): void  { __native__b2d_weld_joint_def_set_angular_damping_ratio(this.handle, d); }
    public function setCollideConnected(bool b): void      { __native__b2d_weld_joint_def_set_collide_connected(this.handle, b); }
}

// ---- Motor joint ----------------------------------------------------------

class MotorJointDef {
    public int handle;
    public constructor() { this.handle = __native__b2d_motor_joint_def_create(); }
    public function destroy(): void { __native__b2d_motor_joint_def_destroy(this.handle); }
    public function setBodies(Body a, Body b): void {
        __native__b2d_motor_joint_def_set_bodies(this.handle, a.handle, b.handle);
    }
    public function setLinearOffset(float x, float y): void { __native__b2d_motor_joint_def_set_linear_offset(this.handle, x, y); }
    public function setAngularOffset(float r): void        { __native__b2d_motor_joint_def_set_angular_offset(this.handle, r); }
    public function setMaxForce(float v): void             { __native__b2d_motor_joint_def_set_max_force(this.handle, v); }
    public function setMaxTorque(float v): void            { __native__b2d_motor_joint_def_set_max_torque(this.handle, v); }
    public function setCorrectionFactor(float v): void     { __native__b2d_motor_joint_def_set_correction_factor(this.handle, v); }
    public function setCollideConnected(bool b): void      { __native__b2d_motor_joint_def_set_collide_connected(this.handle, b); }
}

// ---- Mouse joint ----------------------------------------------------------

class MouseJointDef {
    public int handle;
    public constructor() { this.handle = __native__b2d_mouse_joint_def_create(); }
    public function destroy(): void { __native__b2d_mouse_joint_def_destroy(this.handle); }
    public function setBodies(Body a, Body b): void {
        __native__b2d_mouse_joint_def_set_bodies(this.handle, a.handle, b.handle);
    }
    public function setTarget(float x, float y): void   { __native__b2d_mouse_joint_def_set_target(this.handle, x, y); }
    public function setHertz(float h): void             { __native__b2d_mouse_joint_def_set_hertz(this.handle, h); }
    public function setDampingRatio(float d): void      { __native__b2d_mouse_joint_def_set_damping_ratio(this.handle, d); }
    public function setMaxForce(float v): void          { __native__b2d_mouse_joint_def_set_max_force(this.handle, v); }
    public function setCollideConnected(bool b): void   { __native__b2d_mouse_joint_def_set_collide_connected(this.handle, b); }
}

// ---- Wheel joint ----------------------------------------------------------

class WheelJointDef {
    public int handle;
    public constructor() { this.handle = __native__b2d_wheel_joint_def_create(); }
    public function destroy(): void { __native__b2d_wheel_joint_def_destroy(this.handle); }
    public function setBodies(Body a, Body b): void {
        __native__b2d_wheel_joint_def_set_bodies(this.handle, a.handle, b.handle);
    }
    public function setLocalAnchorA(float x, float y): void { __native__b2d_wheel_joint_def_set_local_anchor_a(this.handle, x, y); }
    public function setLocalAnchorB(float x, float y): void { __native__b2d_wheel_joint_def_set_local_anchor_b(this.handle, x, y); }
    public function setLocalAxisA(float x, float y): void   { __native__b2d_wheel_joint_def_set_local_axis_a(this.handle, x, y); }
    public function setEnableSpring(bool b): void          { __native__b2d_wheel_joint_def_set_enable_spring(this.handle, b); }
    public function setHertz(float h): void                { __native__b2d_wheel_joint_def_set_hertz(this.handle, h); }
    public function setDampingRatio(float d): void         { __native__b2d_wheel_joint_def_set_damping_ratio(this.handle, d); }
    public function setEnableLimit(bool b): void           { __native__b2d_wheel_joint_def_set_enable_limit(this.handle, b); }
    public function setLowerTranslation(float v): void     { __native__b2d_wheel_joint_def_set_lower_translation(this.handle, v); }
    public function setUpperTranslation(float v): void     { __native__b2d_wheel_joint_def_set_upper_translation(this.handle, v); }
    public function setEnableMotor(bool b): void           { __native__b2d_wheel_joint_def_set_enable_motor(this.handle, b); }
    public function setMaxMotorTorque(float v): void       { __native__b2d_wheel_joint_def_set_max_motor_torque(this.handle, v); }
    public function setMotorSpeed(float v): void           { __native__b2d_wheel_joint_def_set_motor_speed(this.handle, v); }
    public function setCollideConnected(bool b): void      { __native__b2d_wheel_joint_def_set_collide_connected(this.handle, b); }
}

// ---- Filter joint (suppresses collision between two specific bodies) -----

class FilterJointDef {
    public int handle;
    public constructor() { this.handle = __native__b2d_filter_joint_def_create(); }
    public function destroy(): void { __native__b2d_filter_joint_def_destroy(this.handle); }
    public function setBodies(Body a, Body b): void {
        __native__b2d_filter_joint_def_set_bodies(this.handle, a.handle, b.handle);
    }
}

// ---- Joint (runtime) ------------------------------------------------------

class Joint {
    public int handle;
    public constructor(int h) { this.handle = h; }

    public function destroy(): void                    { __native__b2d_joint_destroy(this.handle); }
    public function type(): int                        { return __native__b2d_joint_get_type(this.handle); }
    // [bodyIdA, bodyIdB]
    public function bodies(): int[]                    { return __native__b2d_joint_get_bodies(this.handle); }
    public function localAnchorA(): float[]            { return __native__b2d_joint_get_local_anchor_a(this.handle); }
    public function localAnchorB(): float[]            { return __native__b2d_joint_get_local_anchor_b(this.handle); }
    // Sum of constraint forces, in newtons.
    public function constraintForce(): float[]         { return __native__b2d_joint_get_constraint_force(this.handle); }
    public function constraintTorque(): float          { return __native__b2d_joint_get_constraint_torque(this.handle); }
    public function wakeBodies(): void                  { __native__b2d_joint_wake_bodies(this.handle); }

    // ---- distance joint ops ----
    public function distSetLength(float l): void         { __native__b2d_distance_joint_set_length(this.handle, l); }
    public function distLength(): float                  { return __native__b2d_distance_joint_get_length(this.handle); }
    public function distCurrentLength(): float           { return __native__b2d_distance_joint_get_current_length(this.handle); }

    // ---- revolute joint ops ----
    public function revAngle(): float                    { return __native__b2d_revolute_joint_get_angle(this.handle); }
    public function revEnableMotor(bool b): void         { __native__b2d_revolute_joint_enable_motor(this.handle, b); }
    public function revSetMotorSpeed(float s): void      { __native__b2d_revolute_joint_set_motor_speed(this.handle, s); }
    public function revSetMaxMotorTorque(float t): void  { __native__b2d_revolute_joint_set_max_motor_torque(this.handle, t); }
    public function revSetLimits(float lower, float upper): void {
        __native__b2d_revolute_joint_set_limits(this.handle, lower, upper);
    }

    // ---- prismatic joint ops ----
    public function prismTranslation(): float            { return __native__b2d_prismatic_joint_get_translation(this.handle); }
    public function prismSpeed(): float                  { return __native__b2d_prismatic_joint_get_speed(this.handle); }
    public function prismEnableMotor(bool b): void       { __native__b2d_prismatic_joint_enable_motor(this.handle, b); }
    public function prismSetMotorSpeed(float s): void    { __native__b2d_prismatic_joint_set_motor_speed(this.handle, s); }
    public function prismSetMaxMotorForce(float f): void { __native__b2d_prismatic_joint_set_max_motor_force(this.handle, f); }
    public function prismSetLimits(float lower, float upper): void {
        __native__b2d_prismatic_joint_set_limits(this.handle, lower, upper);
    }

    // ---- mouse joint ops ----
    public function mouseSetTarget(float x, float y): void { __native__b2d_mouse_joint_set_target(this.handle, x, y); }
    public function mouseSetMaxForce(float v): void        { __native__b2d_mouse_joint_set_max_force(this.handle, v); }

    // ---- wheel joint ops ----
    public function wheelSetMotorSpeed(float s): void      { __native__b2d_wheel_joint_set_motor_speed(this.handle, s); }
    public function wheelSetMaxMotorTorque(float t): void  { __native__b2d_wheel_joint_set_max_motor_torque(this.handle, t); }
    public function wheelMotorTorque(): float              { return __native__b2d_wheel_joint_get_motor_torque(this.handle); }

    // ---- weld joint ops ----
    public function weldReferenceAngle(): float            { return __native__b2d_weld_joint_get_reference_angle(this.handle); }
    public function weldSetReferenceAngle(float r): void   { __native__b2d_weld_joint_set_reference_angle(this.handle, r); }
}

class Joints {
    public static function createDistance(World w, DistanceJointDef def): Joint {
        return new Joint(__native__b2d_distance_joint_create(w.handle, def.handle));
    }
    public static function createRevolute(World w, RevoluteJointDef def): Joint {
        return new Joint(__native__b2d_revolute_joint_create(w.handle, def.handle));
    }
    public static function createPrismatic(World w, PrismaticJointDef def): Joint {
        return new Joint(__native__b2d_prismatic_joint_create(w.handle, def.handle));
    }
    public static function createWeld(World w, WeldJointDef def): Joint {
        return new Joint(__native__b2d_weld_joint_create(w.handle, def.handle));
    }
    public static function createMotor(World w, MotorJointDef def): Joint {
        return new Joint(__native__b2d_motor_joint_create(w.handle, def.handle));
    }
    public static function createMouse(World w, MouseJointDef def): Joint {
        return new Joint(__native__b2d_mouse_joint_create(w.handle, def.handle));
    }
    public static function createWheel(World w, WheelJointDef def): Joint {
        return new Joint(__native__b2d_wheel_joint_create(w.handle, def.handle));
    }
    public static function createFilter(World w, FilterJointDef def): Joint {
        return new Joint(__native__b2d_filter_joint_create(w.handle, def.handle));
    }
}
