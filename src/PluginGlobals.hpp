#pragma once
/*
 * Plugin-global state shared across the binding TUs:
 *  - host vtable pointer (set in mtype_plugin_register)
 *  - HandleRegistry per plugin-allocated C++ def type (b2BodyDef, b2ShapeDef,
 *    per-joint defs, b2ChainDef, b2WorldDef)
 *  - WorldState map (keyed by packed b2WorldId int)
 *
 * Definitions live in PluginEntry.cpp.
 *
 * Box2D id types (b2WorldId / b2BodyId / b2ShapeId / b2JointId / b2ChainId)
 * are NOT in any registry — they pack into int64 via IdConvert.hpp.
 */

#include "PluginHostApi.h"
#include "HandleRegistry.hpp"
#include "WorldState.hpp"

#include "box2d/box2d.h"
#include "box2d/types.h"

#include <unordered_map>

namespace mtypeb2d
{
    extern const MTypePluginHost* g_host;

    /* Per-world plugin-side state. Key: worldToInt(b2WorldId). */
    extern std::unordered_map<int64_t, WorldState*> g_worldStates;

    /* Plugin-owned def heap objects. Created by *_def_create natives,
     * mutated by *_def_set_*, destroyed by *_def_destroy. */
    extern HandleRegistry<b2WorldDef>           g_worldDefs;
    extern HandleRegistry<b2BodyDef>            g_bodyDefs;
    extern HandleRegistry<b2ShapeDef>           g_shapeDefs;
    extern HandleRegistry<b2ChainDef>           g_chainDefs;

    extern HandleRegistry<b2DistanceJointDef>   g_distanceJointDefs;
    extern HandleRegistry<b2RevoluteJointDef>   g_revoluteJointDefs;
    extern HandleRegistry<b2PrismaticJointDef>  g_prismaticJointDefs;
    extern HandleRegistry<b2WeldJointDef>       g_weldJointDefs;
    extern HandleRegistry<b2MotorJointDef>      g_motorJointDefs;
    extern HandleRegistry<b2MouseJointDef>      g_mouseJointDefs;
    extern HandleRegistry<b2WheelJointDef>      g_wheelJointDefs;
    extern HandleRegistry<b2FilterJointDef>     g_filterJointDefs;

    /* Helper: get-or-null the WorldState for a packed world id. */
    WorldState* findWorldState(int64_t worldIdInt);

    /* Initialize the b2DebugDraw callbacks on a fresh WorldState. Defined
     * in DebugDrawBindings.cpp where the callback bodies live. */
    void initWorldDebugDraw(WorldState& ws);

    /* Helper: create a fresh WorldState for a newly created world and wire
     * its b2DebugDraw context pointer. Implemented in PluginEntry.cpp. */
    WorldState* createWorldState(int64_t worldIdInt);

    /* Helper: tear down a WorldState when its world is destroyed. */
    void destroyWorldState(int64_t worldIdInt);

    /* Subsystem registration forward declarations. */
    void registerWorldNatives(MTypeContext* ctx);
    void registerBodyNatives(MTypeContext* ctx);
    void registerShapeNatives(MTypeContext* ctx);
    void registerJointNatives(MTypeContext* ctx);
    void registerQueryNatives(MTypeContext* ctx);
    void registerEventNatives(MTypeContext* ctx);
    void registerDebugDrawNatives(MTypeContext* ctx);
    void registerDefNatives(MTypeContext* ctx);
    void registerMathNatives(MTypeContext* ctx);
}
