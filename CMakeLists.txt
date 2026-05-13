# mtype-box2d — runtime plugin exposing Box2D v3 to mType.
#
# Builds mtype_b2d.{dll,so,dylib}, loaded by mType scripts via
# __plugin_load. Depends only on the C ABI declared in
# include/PluginHostApi.h (vendored — keep in sync with mType if
# MTYPE_PLUGIN_ABI_VERSION bumps) and vendor/box2d (3.x submodule).
#
# Box2D is statically linked into the plugin so only one DLL ships.

cmake_minimum_required(VERSION 3.16)
project(mtype_b2d CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# Force dynamic CRT (/MD) on MSVC so Box2D and the plugin share the same
# runtime — mixing /MT and /MD inside one DLL triggers LNK4098 and corrupts
# static state (e.g. errno).
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL" CACHE STRING "" FORCE)

# ---- vendor: Box2D v3 ------------------------------------------------------
# Static link Box2D into the plugin so deployment is a single DLL. Box2D's
# own CMake project respects BUILD_SHARED_LIBS, so we save/restore it
# locally — the plugin itself must remain a SHARED library.

set(BOX2D_SAMPLES        OFF CACHE BOOL "" FORCE)
set(BOX2D_UNIT_TESTS     OFF CACHE BOOL "" FORCE)
set(BOX2D_BENCHMARKS     OFF CACHE BOOL "" FORCE)
set(BOX2D_DOCS           OFF CACHE BOOL "" FORCE)
set(BOX2D_VALIDATE       OFF CACHE BOOL "" FORCE)

set(_prev_shared ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(vendor/box2d EXCLUDE_FROM_ALL)
set(BUILD_SHARED_LIBS ${_prev_shared})

# Box2D needs PIC when linked into a shared object on Linux.
set_property(TARGET box2d PROPERTY POSITION_INDEPENDENT_CODE ON)

# ---- the plugin shared lib -------------------------------------------------
set(PLUGIN_SOURCES
    src/PluginEntry.cpp
    src/DefBindings.cpp
    src/WorldBindings.cpp
    src/BodyBindings.cpp
    src/ShapeBindings.cpp
    src/JointBindings.cpp
    src/QueryBindings.cpp
    src/EventBindings.cpp
    src/DebugDrawBindings.cpp
    src/MathBindings.cpp
    src/PluginGlobals.hpp
    src/HandleRegistry.hpp
    src/BindingHelpers.hpp
    src/IdConvert.hpp
    src/WorldState.hpp
)

add_library(mtype_b2d SHARED ${PLUGIN_SOURCES})

target_include_directories(mtype_b2d PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/vendor/box2d/include
)

target_link_libraries(mtype_b2d PRIVATE box2d)

set_target_properties(mtype_b2d PROPERTIES
    PREFIX ""
    OUTPUT_NAME "mtype_b2d"
)

# Linux: visibility hidden by default — only the MTYPE_PLUGIN_EXPORT
# symbol leaks. Windows opts in via __declspec(dllexport).
if(UNIX AND NOT APPLE)
    set_target_properties(mtype_b2d PROPERTIES
        SUFFIX ".so"
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )
endif()
