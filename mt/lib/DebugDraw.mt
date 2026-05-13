// Debug-draw readout helpers. After calling world.draw() the plugin's
// scratch buffers contain the world's debug geometry — iterate via these
// helpers and forward to any renderer (e.g. mtype-sfml's draw primitives).

import * from "Box2D.mt";

class DebugDraw {
    public static function polygonCount(World w): int {
        return __native__b2d_debug_get_polygon_count(w.handle);
    }
    // [vertCount, color, x0, y0, x1, y1, ...] — vertCount and color are
    // returned as floats; cast to int after reading.
    public static function polygon(World w, int idx): float[] {
        return __native__b2d_debug_get_polygon(w.handle, idx);
    }

    public static function circleCount(World w): int {
        return __native__b2d_debug_get_circle_count(w.handle);
    }
    // [cx, cy, radius, color]
    public static function circle(World w, int idx): float[] {
        return __native__b2d_debug_get_circle(w.handle, idx);
    }

    public static function segmentCount(World w): int {
        return __native__b2d_debug_get_segment_count(w.handle);
    }
    // [x1, y1, x2, y2, color]
    public static function segment(World w, int idx): float[] {
        return __native__b2d_debug_get_segment(w.handle, idx);
    }

    public static function transformCount(World w): int {
        return __native__b2d_debug_get_transform_count(w.handle);
    }
    // [px, py, cos, sin]
    public static function transform(World w, int idx): float[] {
        return __native__b2d_debug_get_transform(w.handle, idx);
    }
}
