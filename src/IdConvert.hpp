#pragma once
/*
 * Box2D v3 id-struct <-> int64 packing.
 *
 * Box2D v3 ids are small value structs with embedded generation/revision bits.
 * They are designed for cross-language interop: every id type (except
 * b2WorldId — which packs into uint16) ships with b2StoreXxxId/b2LoadXxxId
 * helpers. mType sees these packed values as plain int64 handles.
 *
 * Validity is checked against the live world via b2World_IsValid /
 * b2Body_IsValid / b2Shape_IsValid / b2Joint_IsValid / b2Chain_IsValid —
 * the revision bits in the packed value make stale-handle detection cheap
 * and exact (no separate registry needed).
 *
 * b2WorldId layout: { uint16_t index1; uint16_t generation; } — packs into
 * 32 bits, easily fits in int64. No b2StoreWorldId helper exists in v3.1
 * so we pack manually.
 */

#include <cstdint>

#include "box2d/box2d.h"
#include "box2d/id.h"

namespace mtypeb2d
{
    /* ---- b2WorldId: manually pack two uint16s into the low 32 bits. ---- */

    inline int64_t worldToInt(b2WorldId id)
    {
        return (static_cast<int64_t>(id.generation) << 16) |
               static_cast<int64_t>(id.index1);
    }

    inline b2WorldId intToWorld(int64_t v)
    {
        b2WorldId id{};
        id.index1     = static_cast<uint16_t>(v & 0xFFFF);
        id.generation = static_cast<uint16_t>((v >> 16) & 0xFFFF);
        return id;
    }

    /* ---- b2BodyId / b2ShapeId / b2JointId / b2ChainId via Box2D helpers ---- */

    inline int64_t bodyToInt(b2BodyId id)   { return static_cast<int64_t>(b2StoreBodyId(id)); }
    inline b2BodyId intToBody(int64_t v)    { return b2LoadBodyId(static_cast<uint64_t>(v)); }

    inline int64_t shapeToInt(b2ShapeId id) { return static_cast<int64_t>(b2StoreShapeId(id)); }
    inline b2ShapeId intToShape(int64_t v)  { return b2LoadShapeId(static_cast<uint64_t>(v)); }

    inline int64_t jointToInt(b2JointId id) { return static_cast<int64_t>(b2StoreJointId(id)); }
    inline b2JointId intToJoint(int64_t v)  { return b2LoadJointId(static_cast<uint64_t>(v)); }

    inline int64_t chainToInt(b2ChainId id) { return static_cast<int64_t>(b2StoreChainId(id)); }
    inline b2ChainId intToChain(int64_t v)  { return b2LoadChainId(static_cast<uint64_t>(v)); }
}
