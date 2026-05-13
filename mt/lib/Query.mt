// Query helpers. AABB overlap, ray cast.
//
// All filters are (categoryBits, maskBits) — set both to ~0 / -1 to match
// everything (Box2D defaults are 1 and UINT64_MAX respectively).

import * from "Box2D.mt";

class Query {
    // Shape ids whose AABB overlaps the given world-space box. Filter matches
    // shapes whose `categoryBits & maskBits` is non-zero with the filter.
    public static function overlapAABB(World w,
                                        float minX, float minY,
                                        float maxX, float maxY,
                                        int categoryBits, int maskBits): int[] {
        return __native__b2d_world_overlap_aabb(w.handle,
            minX, minY, maxX, maxY, categoryBits, maskBits);
    }

    // Closest ray hit, or empty array if none.
    // Returns [hit=1, shapeId, px, py, nx, ny, fraction] when something was hit.
    public static function castRayClosest(World w,
                                           float ox, float oy,
                                           float tx, float ty,
                                           int categoryBits, int maskBits): float[] {
        return __native__b2d_world_cast_ray_closest(w.handle,
            ox, oy, tx, ty, categoryBits, maskBits);
    }

    // All ray hits along the ray. Returns [n, sid0, px0, py0, nx0, ny0, frac0,
    // sid1, px1, py1, nx1, ny1, frac1, ...]. Each hit is 6 floats; the first
    // float is the hit count.
    public static function castRayAll(World w,
                                       float ox, float oy,
                                       float tx, float ty,
                                       int categoryBits, int maskBits): float[] {
        return __native__b2d_world_cast_ray_all(w.handle,
            ox, oy, tx, ty, categoryBits, maskBits);
    }
}
