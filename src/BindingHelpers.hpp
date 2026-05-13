#pragma once
/*
 * Helpers shared by the binding TUs. Each .cpp picks its own exception-type
 * string (e.g. "Box2DError") via a kEx constant in its anonymous namespace so
 * call sites stay terse:
 *
 *   if (!requireArgs(ctx, argc, 3, "__native__foo", kEx)) return ...;
 *   sf::RenderWindow* w = findOrRaise(g_windows, g_host->getInt(args[0]),
 *                                     ctx, "__native__foo", kEx);
 *   if (!w) return ...;
 */

#include "PluginGlobals.hpp"
#include "HandleRegistry.hpp"
#include "IdConvert.hpp"

#include "box2d/math_functions.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace mtypeb2d::detail
{
    inline bool requireArgs(MTypeContext* ctx, int argc, int expected,
                            const char* name, const char* exType)
    {
        if (argc != expected) {
            std::string m = std::string(name) + ": expected " + std::to_string(expected)
                          + " args, got " + std::to_string(argc);
            g_host->raiseError(ctx, exType, m.c_str());
            return false;
        }
        return true;
    }

    inline const char* getStr(const MTypeValue* v, std::size_t* outLen = nullptr)
    {
        if (g_host->getTag(v) != MT_TAG_STRING) {
            if (outLen) *outLen = 0;
            return "";
        }
        return g_host->getString(v, outLen);
    }

    /* Scalar extraction shorthands. The host vtable exposes ints as int64
     * and floats as double; binding code almost always wants the narrower
     * Box2D-side types (float, int, bool). */
    inline float    getF (const MTypeValue* v) { return static_cast<float>(g_host->getFloat(v)); }
    inline int      getI (const MTypeValue* v) { return static_cast<int>(g_host->getInt(v)); }
    inline uint32_t getU (const MTypeValue* v) { return static_cast<uint32_t>(g_host->getInt(v)); }
    inline uint64_t getU64(const MTypeValue* v) { return static_cast<uint64_t>(g_host->getInt(v)); }
    inline bool     getB (const MTypeValue* v) { return g_host->getBool(v) != 0; }

    inline b2Vec2   getVec2(const MTypeValue* x, const MTypeValue* y)
    {
        return b2Vec2{ getF(x), getF(y) };
    }

    /* Result builders for the common "array return" shapes. */
    inline MTypeValue* makeIntPair(MTypeContext* ctx, std::int64_t a, std::int64_t b)
    {
        MTypeValue* out = g_host->makeArray(ctx, MT_TAG_INT, 2);
        g_host->arraySet(out, 0, g_host->makeInt(ctx, a));
        g_host->arraySet(out, 1, g_host->makeInt(ctx, b));
        return out;
    }
    inline MTypeValue* makeFloatPair(MTypeContext* ctx, double x, double y)
    {
        MTypeValue* out = g_host->makeArray(ctx, MT_TAG_FLOAT, 2);
        g_host->arraySet(out, 0, g_host->makeFloat(ctx, x));
        g_host->arraySet(out, 1, g_host->makeFloat(ctx, y));
        return out;
    }
    inline MTypeValue* makeFloatTriple(MTypeContext* ctx, double x, double y, double z)
    {
        MTypeValue* out = g_host->makeArray(ctx, MT_TAG_FLOAT, 3);
        g_host->arraySet(out, 0, g_host->makeFloat(ctx, x));
        g_host->arraySet(out, 1, g_host->makeFloat(ctx, y));
        g_host->arraySet(out, 2, g_host->makeFloat(ctx, z));
        return out;
    }
    inline MTypeValue* makeFloatQuad(MTypeContext* ctx,
                                       double x, double y, double z, double w)
    {
        MTypeValue* out = g_host->makeArray(ctx, MT_TAG_FLOAT, 4);
        g_host->arraySet(out, 0, g_host->makeFloat(ctx, x));
        g_host->arraySet(out, 1, g_host->makeFloat(ctx, y));
        g_host->arraySet(out, 2, g_host->makeFloat(ctx, z));
        g_host->arraySet(out, 3, g_host->makeFloat(ctx, w));
        return out;
    }
    inline MTypeValue* makeIntQuad(MTypeContext* ctx,
                                     std::int64_t a, std::int64_t b,
                                     std::int64_t c, std::int64_t d)
    {
        MTypeValue* out = g_host->makeArray(ctx, MT_TAG_INT, 4);
        g_host->arraySet(out, 0, g_host->makeInt(ctx, a));
        g_host->arraySet(out, 1, g_host->makeInt(ctx, b));
        g_host->arraySet(out, 2, g_host->makeInt(ctx, c));
        g_host->arraySet(out, 3, g_host->makeInt(ctx, d));
        return out;
    }
    inline MTypeValue* emptyIntArray(MTypeContext* ctx)
    {
        return g_host->makeArray(ctx, MT_TAG_INT, 0);
    }
    inline MTypeValue* emptyFloatArray(MTypeContext* ctx)
    {
        return g_host->makeArray(ctx, MT_TAG_FLOAT, 0);
    }

    /* Look up `id` in `reg`. On miss, raise an error and return nullptr;
     * the caller should return promptly. */
    template <typename T>
    T* findDefOrRaise(HandleRegistry<T>& reg, std::int64_t id,
                     MTypeContext* ctx, const char* op, const char* exType)
    {
        T* p = reg.find(id);
        if (!p) {
            std::string m = std::string(op) + ": invalid def handle id "
                          + std::to_string(id);
            g_host->raiseError(ctx, exType, m.c_str());
        }
        return p;
    }

    /* Box2D id validity check (uses generation bits — does not need a
     * registry lookup). Raises on stale/null id and returns false. */
    inline bool validateWorld(b2WorldId id, MTypeContext* ctx, const char* op, const char* exType)
    {
        if (!b2World_IsValid(id)) {
            std::string m = std::string(op) + ": invalid world id";
            g_host->raiseError(ctx, exType, m.c_str());
            return false;
        }
        return true;
    }
    inline bool validateBody(b2BodyId id, MTypeContext* ctx, const char* op, const char* exType)
    {
        if (!b2Body_IsValid(id)) {
            std::string m = std::string(op) + ": invalid body id";
            g_host->raiseError(ctx, exType, m.c_str());
            return false;
        }
        return true;
    }
    inline bool validateShape(b2ShapeId id, MTypeContext* ctx, const char* op, const char* exType)
    {
        if (!b2Shape_IsValid(id)) {
            std::string m = std::string(op) + ": invalid shape id";
            g_host->raiseError(ctx, exType, m.c_str());
            return false;
        }
        return true;
    }
    inline bool validateJoint(b2JointId id, MTypeContext* ctx, const char* op, const char* exType)
    {
        if (!b2Joint_IsValid(id)) {
            std::string m = std::string(op) + ": invalid joint id";
            g_host->raiseError(ctx, exType, m.c_str());
            return false;
        }
        return true;
    }
    inline bool validateChain(b2ChainId id, MTypeContext* ctx, const char* op, const char* exType)
    {
        if (!b2Chain_IsValid(id)) {
            std::string m = std::string(op) + ": invalid chain id";
            g_host->raiseError(ctx, exType, m.c_str());
            return false;
        }
        return true;
    }

    /* Batched name-prefix registrar. Collapses long `reg("__native__b2d_xxx", &fn)`
     * tables into chained calls:
     *
     *   Registrar r{ctx, "__native__b2d_"};
     *   r("world_create",  &nWorldCreate)
     *    ("world_destroy", &nWorldDestroy);
     */
    struct Registrar
    {
        MTypeContext* ctx;
        const char*   prefix;

        Registrar& operator()(const char* suffix, MTypeNativeFn fn)
        {
            std::string name = std::string(prefix) + suffix;
            g_host->registerFunction(ctx, name.c_str(), fn, nullptr);
            return *this;
        }
    };
}
