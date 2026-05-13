// mType wrappers around the Box2D v3 natives exposed by mtype-box2d.
// Top-level World class, factory entry points, and the stable enums and
// event-tag constants. Body/Shape/Joint/Query/DebugDraw wrappers live in
// sibling .mt files.

class World {
    public int handle;

    public constructor(int h) { this.handle = h; }

    public function destroy(): void { __native__b2d_world_destroy(this.handle); }

    // Advance simulation by `dt` seconds using `subSteps` sub-iterations
    // (4-8 is typical). Snapshots event buffers for subsequent drains.
    public function step(float dt, int subSteps): void {
        __native__b2d_world_step(this.handle, dt, subSteps);
    }

    public function setGravity(float x, float y): void {
        __native__b2d_world_set_gravity(this.handle, x, y);
    }
    public function gravity(): float[] {
        return __native__b2d_world_get_gravity(this.handle);
    }

    public function enableSleeping(bool flag): void {
        __native__b2d_world_enable_sleeping(this.handle, flag);
    }
    public function enableContinuous(bool flag): void {
        __native__b2d_world_enable_continuous(this.handle, flag);
    }
    public function setRestitutionThreshold(float v): void {
        __native__b2d_world_set_restitution_threshold(this.handle, v);
    }
    public function setHitEventThreshold(float v): void {
        __native__b2d_world_set_hit_event_threshold(this.handle, v);
    }
    public function awakeBodyCount(): int {
        return __native__b2d_world_get_awake_body_count(this.handle);
    }

    // Apply a circular explosion impulse to bodies in radius. Affects
    // circles, capsules, and polygons; ignores segments and chains.
    public function explode(float x, float y, float radius, float falloff,
                              float impulsePerLength): void {
        __native__b2d_world_explode(this.handle, x, y, radius, falloff, impulsePerLength);
    }

    // ---- event drains. Empty array = drained for this step. ----

    // [shapeIdA, shapeIdB]
    public function nextContactBegin(): int[] {
        return __native__b2d_world_next_contact_begin(this.handle);
    }
    public function nextContactEnd(): int[] {
        return __native__b2d_world_next_contact_end(this.handle);
    }
    // [shapeIdA, shapeIdB, px, py, nx, ny, approachSpeed]
    public function nextContactHit(): float[] {
        return __native__b2d_world_next_contact_hit(this.handle);
    }
    // [sensorShapeId, visitorShapeId]
    public function nextSensorBegin(): int[] {
        return __native__b2d_world_next_sensor_begin(this.handle);
    }
    public function nextSensorEnd(): int[] {
        return __native__b2d_world_next_sensor_end(this.handle);
    }
    // [bodyId, px, py, cos, sin, fellAsleep]
    public function nextBodyMove(): float[] {
        return __native__b2d_world_next_body_move(this.handle);
    }
    // [contactBegin, contactEnd, contactHit, sensorBegin, sensorEnd, bodyMove]
    public function eventCounts(): int[] {
        return __native__b2d_world_event_counts(this.handle);
    }
    public function resetEventCursors(): void {
        __native__b2d_world_reset_event_cursors(this.handle);
    }

    // ---- debug draw. Call once per frame between step() and reading
    // back the polygon/circle/segment lists via DebugDraw.mt helpers. ----

    public function draw(): void { __native__b2d_world_draw(this.handle); }

    public function setDebugDrawFlags(bool shapes, bool joints, bool bounds,
                                       bool mass, bool contacts, bool contactNormals): void {
        __native__b2d_debug_draw_set_flags(this.handle, shapes, joints, bounds,
                                            mass, contacts, contactNormals);
    }
}

// Builder for a customized world. After tuning, call Box2D::createWorldFromDef.
class WorldDef {
    public int handle;
    public constructor() { this.handle = __native__b2d_world_def_create(); }
    public function destroy(): void { __native__b2d_world_def_destroy(this.handle); }
    public function setGravity(float x, float y): void {
        __native__b2d_world_def_set_gravity_x(this.handle, x);
        __native__b2d_world_def_set_gravity_y(this.handle, y);
    }
    public function setRestitutionThreshold(float v): void { __native__b2d_world_def_set_restitution_threshold(this.handle, v); }
    public function setHitEventThreshold(float v): void    { __native__b2d_world_def_set_hit_event_threshold(this.handle, v); }
    public function setMaximumLinearSpeed(float v): void   { __native__b2d_world_def_set_maximum_linear_speed(this.handle, v); }
    public function enableSleep(bool b): void              { __native__b2d_world_def_set_enable_sleep(this.handle, b); }
    public function enableContinuous(bool b): void         { __native__b2d_world_def_set_enable_continuous(this.handle, b); }
}

class Box2D {
    // Quick-create: World with gravity (gx, gy) and Box2D defaults.
    public static function createWorld(float gx, float gy): World {
        return new World(__native__b2d_world_create(gx, gy));
    }
    public static function createWorldFromDef(WorldDef def): World {
        return new World(__native__b2d_world_create_from_def(def.handle));
    }

    public static function radians(float degrees): float {
        return __native__b2d_math_radians_from_degrees(degrees);
    }
    public static function degrees(float radians): float {
        return __native__b2d_math_degrees_from_radians(radians);
    }
}

// Stable body-type constants. Don't reorder; scripts may hard-code them.
class BodyType {
    public static function staticBody():    int { return 0; }
    public static function kinematicBody(): int { return 1; }
    public static function dynamicBody():   int { return 2; }
}

// Shape kind enum, matching b2ShapeType. Returned by Shape.type().
class ShapeType {
    public static function circle():       int { return 0; }
    public static function capsule():      int { return 1; }
    public static function segment():      int { return 2; }
    public static function polygon():      int { return 3; }
    public static function chainSegment(): int { return 4; }
}

// Joint kind enum, matching b2JointType. Returned by Joint.type().
class JointType {
    public static function distance():  int { return 0; }
    public static function filter():    int { return 1; }
    public static function motor():     int { return 2; }
    public static function mouse():     int { return 3; }
    public static function prismatic(): int { return 4; }
    public static function revolute():  int { return 5; }
    public static function weld():      int { return 6; }
    public static function wheel():     int { return 7; }
}
