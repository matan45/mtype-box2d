/*
 * Plugin entry point. Validates the ABI version, stashes the host vtable,
 * and dispatches to every subsystem's register*Natives() in turn. Defines
 * all plugin-global state: host pointer, def registries, and the per-world
 * WorldState map.
 *
 * Loaded via `__plugin_load("mtype_b2d.dll")` from mType code.
 */

#include "PluginGlobals.hpp"

#include "box2d/box2d.h"

#include <unordered_map>

namespace mtypeb2d
{
    const MTypePluginHost* g_host = nullptr;

    std::unordered_map<int64_t, WorldState*> g_worldStates;

    HandleRegistry<b2WorldDef>           g_worldDefs;
    HandleRegistry<b2BodyDef>            g_bodyDefs;
    HandleRegistry<b2ShapeDef>           g_shapeDefs;
    HandleRegistry<b2ChainDef>           g_chainDefs;

    HandleRegistry<b2DistanceJointDef>   g_distanceJointDefs;
    HandleRegistry<b2RevoluteJointDef>   g_revoluteJointDefs;
    HandleRegistry<b2PrismaticJointDef>  g_prismaticJointDefs;
    HandleRegistry<b2WeldJointDef>       g_weldJointDefs;
    HandleRegistry<b2MotorJointDef>      g_motorJointDefs;
    HandleRegistry<b2MouseJointDef>      g_mouseJointDefs;
    HandleRegistry<b2WheelJointDef>      g_wheelJointDefs;
    HandleRegistry<b2FilterJointDef>     g_filterJointDefs;

    WorldState* findWorldState(int64_t worldIdInt)
    {
        auto it = g_worldStates.find(worldIdInt);
        return it == g_worldStates.end() ? nullptr : it->second;
    }

    WorldState* createWorldState(int64_t worldIdInt)
    {
        auto* ws = new WorldState();
        initWorldDebugDraw(*ws);
        g_worldStates[worldIdInt] = ws;
        return ws;
    }

    void destroyWorldState(int64_t worldIdInt)
    {
        auto it = g_worldStates.find(worldIdInt);
        if (it == g_worldStates.end()) return;
        delete it->second;
        g_worldStates.erase(it);
    }
}

extern "C" MTYPE_PLUGIN_EXPORT
int mtype_plugin_register(uint32_t hostAbiVersion,
                          const MTypePluginHost* host,
                          MTypeContext* registrationCtx)
{
    if (hostAbiVersion != MTYPE_PLUGIN_ABI_VERSION) {
        return 1;
    }
    mtypeb2d::g_host = host;

    mtypeb2d::registerDefNatives(registrationCtx);
    mtypeb2d::registerWorldNatives(registrationCtx);
    mtypeb2d::registerBodyNatives(registrationCtx);
    mtypeb2d::registerShapeNatives(registrationCtx);
    mtypeb2d::registerJointNatives(registrationCtx);
    mtypeb2d::registerQueryNatives(registrationCtx);
    mtypeb2d::registerEventNatives(registrationCtx);
    mtypeb2d::registerDebugDrawNatives(registrationCtx);
    mtypeb2d::registerMathNatives(registrationCtx);
    return 0;
}
