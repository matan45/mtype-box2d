// Headless smoke test for mtype-box2d. No SFML, no rendering.
//
// Drop a single dynamic body from y=0 and step the world 60 times at 1/60s.
// Expected final y ~= 0.5 * 9.8 * 1.0 = 4.9 (Box2D Y points "down" with
// positive gravity here). Prints the final position and velocity.

import * from "../lib/Box2D.mt";
import * from "../lib/Body.mt";
import * from "../lib/Shape.mt";

__plugin_load("mt/mtype_b2d.dll");

World world = Box2D::createWorld(0.0, 9.8);

BodyDef bd = new BodyDef();
bd.setType(BodyType::dynamicBody());
bd.setPosition(0.0, 0.0);
Body box = Bodies::create(world, bd);
bd.destroy();

ShapeDef sd = new ShapeDef();
sd.setDensity(1.0);
Shape shape = Shapes::createBox(box, sd, 0.5, 0.5);
sd.destroy();

// Box2D v3 expects positive gravity to be "down" in the body frame here.
int i = 0;
while (i < 60) {
    world.step(0.016666, 4);
    i = i + 1;
}

float[] p = box.position();
float[] v = box.linearVelocity();
print("final position: x=" + p[0] + ", y=" + p[1]);
print("final velocity: vx=" + v[0] + ", vy=" + v[1]);
print("(expected y ~= 4.9 m, vy ~= 9.8 m/s after 1 s of free fall)");

shape.destroy(true);
box.destroy();
world.destroy();
__plugin_unload("mt/mtype_b2d.dll");
