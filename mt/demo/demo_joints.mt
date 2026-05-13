// Headless test of a revolute-joint rope: 5 dynamic boxes hung off a static
// anchor by 4 revolute joints. Steps the world for 2 seconds and prints
// the final positions to confirm the chain swings as expected under gravity.

import * from "../lib/Box2D.mt";
import * from "../lib/Body.mt";
import * from "../lib/Shape.mt";
import * from "../lib/Joint.mt";

__plugin_load("mt/mtype_b2d.dll");

World world = Box2D::createWorld(0.0, 9.8);

// Anchor: a static body at the origin.
BodyDef ad = new BodyDef();
ad.setType(BodyType::staticBody());
ad.setPosition(0.0, 0.0);
Body anchor = Bodies::create(world, ad);
ad.destroy();

// Chain of 5 dynamic links, each 0.5m wide, hanging vertically below the anchor.
int N = 5;
Body[] links = new Body[N];
int i = 0;
while (i < N) {
    BodyDef ld = new BodyDef();
    ld.setType(BodyType::dynamicBody());
    ld.setPosition(0.5 + i * 1.0, 0.0);  // offset to the right, horizontal chain
    Body lk = Bodies::create(world, ld);
    ld.destroy();

    ShapeDef sd = new ShapeDef();
    sd.setDensity(1.0);
    sd.setFriction(0.2);
    Shape sh = Shapes::createBox(lk, sd, 0.5, 0.1);
    sd.destroy();

    links[i] = lk;
    i = i + 1;
}

// Hinge link[0] to the anchor, then chain link[k] -> link[k+1].
RevoluteJointDef jd = new RevoluteJointDef();
jd.setBodies(anchor, links[0]);
jd.setLocalAnchorA(0.0, 0.0);
jd.setLocalAnchorB(-0.5, 0.0);
Joint j0 = Joints::createRevolute(world, jd);
jd.destroy();

int k = 0;
while (k < N - 1) {
    RevoluteJointDef jdk = new RevoluteJointDef();
    jdk.setBodies(links[k], links[k + 1]);
    jdk.setLocalAnchorA(0.5, 0.0);
    jdk.setLocalAnchorB(-0.5, 0.0);
    Joints::createRevolute(world, jdk);
    jdk.destroy();
    k = k + 1;
}

print("Simulating chain for 2s @ 60Hz...");
int step = 0;
while (step < 120) {
    world.step(0.016666, 4);
    step = step + 1;
}

print("Final link positions (should curve downward under gravity):");
int q = 0;
while (q < N) {
    float[] p = links[q].position();
    print("  link[" + q + "] x=" + p[0] + " y=" + p[1]);
    q = q + 1;
}

world.destroy();
__plugin_unload("mt/mtype_b2d.dll");
