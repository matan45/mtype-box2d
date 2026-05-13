// Demonstrates the v3 contact + hit event poll model.
//
// Two dynamic boxes are placed so they collide. We enable contact events
// and hit events on both. After each step we drain the event buffers and
// print begin/end/hit notifications.
//
// Headless — no SFML. Just stdout.

import * from "../lib/Box2D.mt";
import * from "../lib/Body.mt";
import * from "../lib/Shape.mt";

__plugin_load("mt/mtype_b2d.dll");

World world = Box2D::createWorld(0.0, 9.8);
world.setHitEventThreshold(0.5);  // hit events for collisions over 0.5 m/s

// Floor.
BodyDef gd = new BodyDef();
gd.setType(BodyType::staticBody());
gd.setPosition(0.0, 5.0);
Body ground = Bodies::create(world, gd);
gd.destroy();
ShapeDef gsd = new ShapeDef();
gsd.setFriction(0.5);
gsd.enableContactEvents(true);
Shape gs = Shapes::createSegment(ground, gsd, -5.0, 0.0, 5.0, 0.0);
gsd.destroy();

// Falling box A.
BodyDef bdA = new BodyDef();
bdA.setType(BodyType::dynamicBody());
bdA.setPosition(0.0, -3.0);
Body boxA = Bodies::create(world, bdA);
bdA.destroy();
ShapeDef sdA = new ShapeDef();
sdA.setDensity(1.0);
sdA.enableContactEvents(true);
sdA.enableHitEvents(true);
Shape shapeA = Shapes::createBox(boxA, sdA, 0.5, 0.5);
sdA.destroy();

print("simulating 2 seconds at 60Hz, draining events each step...");

int step = 0;
while (step < 120) {
    world.step(0.016666, 4);

    int[] e = world.nextContactBegin();
    while (e.length > 0) {
        print("[" + step + "] CONTACT BEGIN  shapeA=" + e[0] + " shapeB=" + e[1]);
        e = world.nextContactBegin();
    }
    float[] h = world.nextContactHit();
    while (h.length > 0) {
        print("[" + step + "] CONTACT HIT  point=(" + h[2] + "," + h[3] + ") speed=" + h[6]);
        h = world.nextContactHit();
    }
    int[] ee = world.nextContactEnd();
    while (ee.length > 0) {
        print("[" + step + "] CONTACT END  shapeA=" + ee[0] + " shapeB=" + ee[1]);
        ee = world.nextContactEnd();
    }
    step = step + 1;
}

int[] counts = world.eventCounts();
print("final last-step counts: cb=" + counts[0] + " ce=" + counts[1]
       + " ch=" + counts[2] + " sb=" + counts[3] + " se=" + counts[4]
       + " bm=" + counts[5]);

shapeA.destroy(true);
boxA.destroy();
gs.destroy(false);
ground.destroy();
world.destroy();
__plugin_unload("mt/mtype_b2d.dll");
