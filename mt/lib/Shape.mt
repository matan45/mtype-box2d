// Shape wrappers. Box / circle / polygon / capsule / segment / chain.
//
// v3 NOTE: contact, hit, and sensor events are OFF by default per-shape.
// Call enableContactEvents(true) / enableHitEvents(true) / enableSensorEvents(true)
// on shapes you want to surface in event drains.

import * from "Box2D.mt";
import * from "Body.mt";

// Builder for a shape. Configure material, density, filter, and event
// opt-ins, then pass to one of the Shapes::createXxx factories.
class ShapeDef {
    public int handle;
    public constructor() { this.handle = __native__b2d_shape_def_create(); }
    public function destroy(): void { __native__b2d_shape_def_destroy(this.handle); }

    public function setDensity(float d): void           { __native__b2d_shape_def_set_density(this.handle, d); }
    public function setFriction(float f): void          { __native__b2d_shape_def_set_friction(this.handle, f); }
    public function setRestitution(float r): void       { __native__b2d_shape_def_set_restitution(this.handle, r); }
    public function setRollingResistance(float r): void  { __native__b2d_shape_def_set_rolling_resistance(this.handle, r); }
    public function setTangentSpeed(float v): void      { __native__b2d_shape_def_set_tangent_speed(this.handle, v); }
    public function setIsSensor(bool b): void           { __native__b2d_shape_def_set_is_sensor(this.handle, b); }
    public function setFilter(int categoryBits, int maskBits, int groupIndex): void {
        __native__b2d_shape_def_set_filter(this.handle, categoryBits, maskBits, groupIndex);
    }
    public function enableSensorEvents(bool b): void    { __native__b2d_shape_def_set_enable_sensor_events(this.handle, b); }
    public function enableContactEvents(bool b): void   { __native__b2d_shape_def_set_enable_contact_events(this.handle, b); }
    public function enableHitEvents(bool b): void       { __native__b2d_shape_def_set_enable_hit_events(this.handle, b); }
    public function enablePreSolveEvents(bool b): void  { __native__b2d_shape_def_set_enable_pre_solve_events(this.handle, b); }
    public function setUpdateBodyMass(bool b): void     { __native__b2d_shape_def_set_update_body_mass(this.handle, b); }
}

class Shape {
    public int handle;
    public constructor(int h) { this.handle = h; }
    public function destroy(bool updateBodyMass): void {
        __native__b2d_shape_destroy(this.handle, updateBodyMass);
    }

    public function type(): int   { return __native__b2d_shape_get_type(this.handle); }
    public function body(): int   { return __native__b2d_shape_get_body(this.handle); }

    public function setDensity(float d, bool updateBodyMass): void {
        __native__b2d_shape_set_density(this.handle, d, updateBodyMass);
    }
    public function setFriction(float f): void    { __native__b2d_shape_set_friction(this.handle, f); }
    public function setRestitution(float r): void { __native__b2d_shape_set_restitution(this.handle, r); }
    public function density(): float    { return __native__b2d_shape_get_density(this.handle); }
    public function friction(): float   { return __native__b2d_shape_get_friction(this.handle); }
    public function restitution(): float { return __native__b2d_shape_get_restitution(this.handle); }

    public function isSensor(): bool    { return __native__b2d_shape_is_sensor(this.handle); }
    public function setFilter(int category, int mask, int group): void {
        __native__b2d_shape_set_filter(this.handle, category, mask, group);
    }
    // [categoryBits, maskBits, groupIndex]
    public function filter(): int[]     { return __native__b2d_shape_get_filter(this.handle); }

    public function enableSensorEvents(bool b): void   { __native__b2d_shape_enable_sensor_events(this.handle, b); }
    public function enableContactEvents(bool b): void  { __native__b2d_shape_enable_contact_events(this.handle, b); }
    public function enableHitEvents(bool b): void      { __native__b2d_shape_enable_hit_events(this.handle, b); }
    public function enablePreSolveEvents(bool b): void { __native__b2d_shape_enable_pre_solve_events(this.handle, b); }

    // [minX, minY, maxX, maxY]
    public function aabb(): float[]     { return __native__b2d_shape_get_aabb(this.handle); }

    // ---- geometry readback (use type() to pick the right one) ----
    // [cx, cy, radius]
    public function asCircle(): float[]  { return __native__b2d_shape_get_circle(this.handle); }
    // [x1, y1, x2, y2]
    public function asSegment(): float[] { return __native__b2d_shape_get_segment(this.handle); }
    // [x0, y0, x1, y1, ...] world-space-relative-to-body vertices
    public function asPolygon(): float[] { return __native__b2d_shape_get_polygon(this.handle); }
    // [c1x, c1y, c2x, c2y, radius]
    public function asCapsule(): float[] { return __native__b2d_shape_get_capsule(this.handle); }

    public function testPoint(float x, float y): bool {
        return __native__b2d_shape_test_point(this.handle, x, y);
    }
}

class Shapes {
    public static function createBox(Body b, ShapeDef def, float halfWidth, float halfHeight): Shape {
        return new Shape(__native__b2d_shape_create_box(b.handle, def.handle, halfWidth, halfHeight));
    }
    public static function createCircle(Body b, ShapeDef def, float radius, float cx, float cy): Shape {
        return new Shape(__native__b2d_shape_create_circle(b.handle, def.handle, radius, cx, cy));
    }
    // verts is a flat float[] of [x0, y0, x1, y1, ...]. At least 3 points.
    // The hull is computed by Box2D; coincident or degenerate points fail.
    public static function createPolygon(Body b, ShapeDef def, float[] verts): Shape {
        return new Shape(__native__b2d_shape_create_polygon(b.handle, def.handle, verts));
    }
    public static function createCapsule(Body b, ShapeDef def,
                                          float x1, float y1, float x2, float y2,
                                          float radius): Shape {
        return new Shape(__native__b2d_shape_create_capsule(b.handle, def.handle,
                                                              x1, y1, x2, y2, radius));
    }
    public static function createSegment(Body b, ShapeDef def,
                                          float x1, float y1, float x2, float y2): Shape {
        return new Shape(__native__b2d_shape_create_segment(b.handle, def.handle,
                                                              x1, y1, x2, y2));
    }
}

// A chain shape is a counter-clockwise loop / strip of segments. Returns
// a handle distinct from individual shapes; resolve the per-segment
// shape ids via segments() if you need to enumerate them.
class Chain {
    public int handle;
    public constructor(int h) { this.handle = h; }
    public function destroy(): void { __native__b2d_chain_destroy(this.handle); }
    public function segments(): int[] { return __native__b2d_chain_get_segments(this.handle); }
}

class Chains {
    public static function create(Body b, float[] points, bool isLoop,
                                  float friction, float restitution): Chain {
        return new Chain(__native__b2d_chain_create(b.handle, points, isLoop,
                                                       friction, restitution));
    }
}
