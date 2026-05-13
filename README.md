# mtype-box2d

[Box2D v3](https://box2d.org) physics for the [mType](https://github.com/matan-grinberg/mType) language, packaged as a runtime plugin loaded via `__plugin_load`.

Mirrors the design of the sibling [mtype-sfml](../mtype-sfml) binding so the two plugins compose: simulate bodies through `mtype-box2d`, render them through `mtype-sfml`.

## What's bound

Comprehensive coverage of Box2D v3:

- **Worlds** — create / destroy / step / gravity / sleeping / continuous collision / restitution threshold / hit-event threshold / explosions
- **Bodies** — all 3 types (static/kinematic/dynamic); position, rotation, linear & angular velocity; forces, impulses, torques; mass data; sleep state; bullet flag; fixed-rotation; enable/disable
- **Shapes** — box, circle, polygon (auto-hull), capsule, segment, chain; density, friction, restitution, filter (category/mask/group); per-shape event opt-ins; shape geometry readback
- **Joints** — distance, revolute, prismatic, weld, motor, mouse, wheel, filter; per-type builders, motors, springs, limits, runtime getters
- **Queries** — AABB overlap, ray cast (closest & all hits)
- **Events** — contact begin/end/hit, sensor begin/end, body-move events; poll-from-buffer drain pattern
- **Debug draw** — `b2World_Draw` plumbing returning polygons/circles/segments/transforms as float arrays the script can forward to any renderer
- **Math** — radians/degrees helpers

## Build

```
git clone --recurse-submodules <this-repo> mtype-box2d
cd mtype-box2d
cmake -B build -S .
cmake --build build --config Release
```

If you already cloned without submodules:

```
git submodule update --init --recursive
```

Output: a single `build/Release/mtype_b2d.dll` (~700 KB on Windows). Box2D is statically linked, no co-shipped DLLs.

Drop the plugin alongside the demos so `__plugin_load` resolves it:

```
cp build/Release/mtype_b2d.dll mt/mtype_b2d.dll
```

## Run

Headless smoke test — drops a body for 1 second, prints final position:

```
mType.exe mt/demo/demo_smoke.mt
```

Expected output:

```
final position: x=0, y=4.92002
final velocity: vx=0, vy=9.79959
```

Other demos:

| File | Purpose |
|---|---|
| `mt/demo/demo_smoke.mt` | Headless 60-step free-fall test, no SFML. |
| `mt/demo/demo_joints.mt` | Revolute-joint chain hung from a static anchor, prints link positions after 2 s. |
| `mt/demo/demo_events.mt` | Two-body collision logging contact begin/hit/end events. |
| `mt/demo/demo_bouncing.mt` | Visual: SFML window + 10 dynamic boxes falling on a floor. Requires `mtype-sfml`. |
| `mt/demo/demo_debug_draw.mt` | Visual: renders `b2World_Draw` output through SFML primitives. Requires `mtype-sfml`. |

The visual demos load **both** plugins. From `mtype-box2d/`:

```
cp <mtype-sfml>/build/Release/mtype_sfml.dll <mtype-sfml>/mt/mtype_sfml.dll
cp <mtype-sfml>/build/Release/sfml-*.dll <mtype-sfml>/mt/   # Windows SFML runtime
mType.exe mt/demo/demo_bouncing.mt
```

## API sketch

The mType-facing surface is in `mt/lib/`. Builder pattern for every constructor — explicit `.destroy()` on the def releases the plugin-side heap object.

```mtype
import * from "mt/lib/Box2D.mt";
import * from "mt/lib/Body.mt";
import * from "mt/lib/Shape.mt";

__plugin_load("mt/mtype_b2d.dll");

World world = Box2D::createWorld(0.0, 9.8);

BodyDef bd = new BodyDef();
bd.setType(BodyType::dynamicBody());
bd.setPosition(0.0, -5.0);
Body box = Bodies::create(world, bd);
bd.destroy();

ShapeDef sd = new ShapeDef();
sd.setDensity(1.0);
sd.setFriction(0.3);
Shape s = Shapes::createBox(box, sd, 0.5, 0.5);
sd.destroy();

world.step(0.016666, 4);
float[] pos = box.position();   // [x, y]
float angle = box.rotation();    // radians

// v3 NOTE: contact / hit / sensor events are OFF by default per-shape.
// Enable them explicitly if you want to drain them:
s.enableContactEvents(true);
s.enableHitEvents(true);

// ...after step():
int[] hit = world.nextContactBegin();   // [shapeIdA, shapeIdB] or []
float[] h = world.nextContactHit();      // [shapeA, shapeB, px, py, nx, ny, speed] or []
```

| File | What's in it |
|---|---|
| `mt/lib/Box2D.mt`     | `World`, `WorldDef`, `Box2D::createWorld` factories, `BodyType`/`ShapeType`/`JointType` enums. |
| `mt/lib/Body.mt`      | `Body`, `BodyDef`, `Bodies::create`. Kinematics, forces, mass, user-data int64 side-channel. |
| `mt/lib/Shape.mt`     | `Shape`, `ShapeDef`, `Shapes::createBox/Circle/Polygon/Capsule/Segment`; `Chain`, `Chains::create`. |
| `mt/lib/Joint.mt`     | `Joint` + one `*JointDef` per joint type; `Joints::createDistance/Revolute/Prismatic/Weld/Motor/Mouse/Wheel/Filter`. |
| `mt/lib/Query.mt`     | `Query::overlapAABB / castRayClosest / castRayAll`. |
| `mt/lib/DebugDraw.mt` | `DebugDraw::polygonCount/polygon/circleCount/circle/segmentCount/segment/...` readout helpers. |

## Architecture notes

- **Handles.** Box2D v3 id structs (`b2WorldId`, `b2BodyId`, `b2ShapeId`, `b2JointId`, `b2ChainId`) pack directly into int64 via `b2Store*Id` / `b2Load*Id`. There are no per-type registries — mType sees these as plain ints, and validity is checked via Box2D's `b2*_IsValid` (revision bits embedded in the packed value).

- **Defs.** Plugin-side def heap objects (one `HandleRegistry<T>` per def type) avoid the need to pass mType class instances across the FFI. The pattern is `new XxxDef()` → setter chain → `Create(world, def)` → `def.destroy()`.

- **Events.** `b2World_Step` invalidates the world's event arrays. We snapshot them into per-world `std::vector`s inside `world.step()` so scripts can drain events lazily; cursors reset per step.

- **Debug draw.** `b2DebugDraw` callbacks fill scratch vectors on a per-world `WorldState`. `world.draw()` clears the scratch, invokes `b2World_Draw`, and the script reads back via `DebugDraw::polygon(i)` etc. The plugin has no SFML dependency — debug geometry is plain float arrays.

- **CRT.** Forced to `/MD` (dynamic CRT) across both Box2D and the plugin to avoid LNK4098 mixed-CRT linkage.

## Source layout

```
mtype-box2d/
├── CMakeLists.txt
├── include/PluginHostApi.h          mType plugin C ABI (vendored from mType)
├── src/
│   ├── PluginEntry.cpp              entry + globals + WorldState map
│   ├── PluginGlobals.hpp            extern decls + WorldState fwd
│   ├── HandleRegistry.hpp           generic int64-id → T* template
│   ├── BindingHelpers.hpp           arg checks, scalar shorthands, result builders
│   ├── IdConvert.hpp                b2*Id ↔ int64 helpers
│   ├── WorldState.hpp               per-world event snapshot + debug-draw scratch
│   ├── DefBindings.cpp              world/body/shape/joint def setters
│   ├── WorldBindings.cpp            world lifecycle + step + gravity
│   ├── BodyBindings.cpp             body operations
│   ├── ShapeBindings.cpp            shape factories + per-shape ops
│   ├── JointBindings.cpp            joint factories + per-type ops
│   ├── QueryBindings.cpp            AABB + ray cast
│   ├── EventBindings.cpp            contact/sensor/body-move iterators
│   ├── DebugDrawBindings.cpp        b2DebugDraw wiring + readout
│   └── MathBindings.cpp             radians/degrees helpers
├── mt/
│   ├── lib/                         mType wrapper classes
│   └── demo/                        sample programs
└── vendor/box2d/                    git submodule (pinned to v3.1.0)
```

## License

The plugin source is under the same license as the mType project. Box2D v3 is MIT-licensed by Erin Catto — see `vendor/box2d/LICENSE`.
