#pragma once
/*
 * Per-world plugin-side state. Keyed by the packed b2WorldId int.
 *
 * Why this exists:
 *   - Box2D v3 event arrays (b2World_Get{Contact,Sensor,Body}Events) are
 *     invalidated by the next b2World_Step. mType might not poll events
 *     immediately, so world_step() snapshots them into per-world vectors
 *     and EventBindings.cpp drains those snapshots via cursors.
 *
 *   - b2DebugDraw callbacks need a per-world context pointer to push vertex
 *     data into. We give them this WorldState.
 *
 *   - b2Body_SetUserData(void*) truncates int64 on 32-bit hosts. mType
 *     promises an int64 user-data slot per body, so we maintain a side-map
 *     keyed by the packed body id.
 *
 * Storage: a global std::unordered_map<int64_t, WorldState*> in
 * PluginEntry.cpp. WorldStates are heap-allocated on world_create and
 * destroyed on world_destroy.
 */

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "box2d/box2d.h"
#include "box2d/types.h"

namespace mtypeb2d
{
    /* Vertex tuple for the per-polygon scratch buffer. b2DebugDraw can emit
     * SolidPolygon (with a transform) and Polygon (already-transformed CCW
     * verts). We collapse both into a single "world-space CCW vertices +
     * color" representation that mType reads back. */
    struct DebugPolygon
    {
        std::vector<float> verts;  /* [x0, y0, x1, y1, ...] in world space */
        uint32_t           color;  /* 0xRRGGBB from b2HexColor */
    };

    struct DebugCircle
    {
        float    cx, cy, radius;
        uint32_t color;
    };

    struct DebugSegment
    {
        float    x1, y1, x2, y2;
        uint32_t color;
    };

    struct DebugTransform
    {
        float px, py, cosA, sinA;
    };

    struct WorldState
    {
        /* ---- snapshotted event buffers, refilled at end of world_step ---- */
        std::vector<b2ContactBeginTouchEvent> contactBegin;
        std::vector<b2ContactEndTouchEvent>   contactEnd;
        std::vector<b2ContactHitEvent>        contactHit;
        std::vector<b2SensorBeginTouchEvent>  sensorBegin;
        std::vector<b2SensorEndTouchEvent>    sensorEnd;
        std::vector<b2BodyMoveEvent>          bodyMove;

        /* cursors — set to 0 after each snapshot; advanced by event-iterator
         * natives in EventBindings.cpp. */
        size_t cbCursor = 0, ceCursor = 0, chCursor = 0;
        size_t sbCursor = 0, seCursor = 0, bmCursor = 0;

        /* ---- debug-draw scratch, refilled at start of world_draw ---- */
        std::vector<DebugPolygon>   polygons;
        std::vector<DebugCircle>    circles;
        std::vector<DebugSegment>   segments;
        std::vector<DebugTransform> transforms;

        b2DebugDraw debugDraw;  /* function pointers + context = this */

        /* Per-body user-data side-map (int64 surrogate for b2Body_SetUserData). */
        std::unordered_map<uint64_t, int64_t> bodyUserData;
    };
}
