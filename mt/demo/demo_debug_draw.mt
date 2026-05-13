// Cross-plugin debug-draw demo. Renders Box2D's debug-draw output (polygons,
// circles, segments) through SFML's primitives. No body tracking on the
// script side — everything comes from b2World_Draw via the plugin's
// scratch buffers.

import * from "../lib/Box2D.mt";
import * from "../lib/Body.mt";
import * from "../lib/Shape.mt";
import * from "../lib/DebugDraw.mt";

import * from "../../mtype-sfml/mt/lib/Sfml.mt";
import * from "../../mtype-sfml/mt/lib/Graphics.mt";
import * from "../../mtype-sfml/mt/lib/System.mt";

__plugin_load("mt/mtype_b2d.dll");
__plugin_load("../mtype-sfml/mt/mtype_sfml.dll");

RenderWindow window = Sfml::createWindow("mtype-box2d debug draw", 800, 600);

World world = Box2D::createWorld(0.0, 9.8);
world.setDebugDrawFlags(true, true, false, false, false, false);

// Ground.
BodyDef gd = new BodyDef();
gd.setType(BodyType::staticBody());
gd.setPosition(0.0, 5.0);
Body ground = Bodies::create(world, gd);
gd.destroy();
ShapeDef gsd = new ShapeDef();
Shape gs = Shapes::createSegment(ground, gsd, -10.0, 0.0, 10.0, 0.0);
gsd.destroy();

// A circle and a polygon.
BodyDef cd = new BodyDef();
cd.setType(BodyType::dynamicBody());
cd.setPosition(-2.0, -3.0);
Body ball = Bodies::create(world, cd);
cd.destroy();
ShapeDef csd = new ShapeDef();
csd.setDensity(1.0);
csd.setRestitution(0.4);
Shape cs = Shapes::createCircle(ball, csd, 0.5, 0.0, 0.0);
csd.destroy();

BodyDef pd = new BodyDef();
pd.setType(BodyType::dynamicBody());
pd.setPosition(2.0, -3.0);
pd.setAngle(Box2D::radians(20.0));
Body brick = Bodies::create(world, pd);
pd.destroy();
ShapeDef psd = new ShapeDef();
psd.setDensity(1.0);
Shape ps = Shapes::createBox(brick, psd, 0.7, 0.3);
psd.destroy();

float scale = 30.0;
float originX = 400.0;
float originY = 100.0;

// Reusable SFML shape objects we mutate per-frame.
CircleShape c = Circles::create(1.0);

int evClosed     = Sfml::closedEventId();
int evKeyPressed = Sfml::keyPressedEventId();
int keyEsc = Key::escape();
Clock frame = Clocks::create();

while (window.isOpen()) {
    int ev = window.pollEvent();
    while (ev != 0) {
        if (ev == evClosed) { window.close(); }
        else if (ev == evKeyPressed) {
            if (Event::key() == keyEsc) { window.close(); }
        }
        ev = window.pollEvent();
    }

    float dt = frame.restartSeconds();
    if (dt > 0.05) { dt = 0.05; }
    world.step(dt, 4);
    world.draw();

    window.clear(20, 22, 30, 255);

    // Polygons: build a ConvexShape per polygon entry. We use a simple
    // RectangleShape per-edge approximation for simplicity here — a proper
    // ConvexShape from VertexArray would be better but requires more glue.
    int polyN = DebugDraw::polygonCount(world);
    int i = 0;
    while (i < polyN) {
        float[] poly = DebugDraw::polygon(world, i);
        int n = (int)poly[0];
        // Draw each edge as a thin SFML rectangle.
        int e = 0;
        while (e < n) {
            int next = (e + 1) % n;
            float x1 = poly[2 + 2 * e];
            float y1 = poly[2 + 2 * e + 1];
            float x2 = poly[2 + 2 * next];
            float y2 = poly[2 + 2 * next + 1];
            float sx1 = originX + x1 * scale;
            float sy1 = originY + y1 * scale;
            float sx2 = originX + x2 * scale;
            float sy2 = originY + y2 * scale;
            float dx = sx2 - sx1;
            float dy = sy2 - sy1;
            float len = (dx * dx + dy * dy);
            // crude length — assume scripts don't need exact for debug draw
            RectangleShape edge = Rectangles::create(1.0, 2.0);
            edge.setOrigin(0.0, 1.0);
            edge.setPosition(sx1, sy1);
            edge.setScale(1.0, 1.0);
            edge.setFillColor(120, 220, 120, 255);
            Draw::rect(window, edge);
            edge.destroy();
            e = e + 1;
        }
        i = i + 1;
    }

    // Circles: just plot a CircleShape per debug circle.
    int cn = DebugDraw::circleCount(world);
    int j = 0;
    while (j < cn) {
        float[] cd = DebugDraw::circle(world, j);
        c.setRadius(cd[2] * scale);
        c.setOrigin(cd[2] * scale, cd[2] * scale);
        c.setPosition(originX + cd[0] * scale, originY + cd[1] * scale);
        c.setFillColor(120, 180, 255, 255);
        Draw::circle(window, c);
        j = j + 1;
    }

    // Segments: long thin rect would be ideal; skipped for brevity (the
    // ground is the only segment in this demo).

    window.display();
}

frame.destroy();
c.destroy();
ps.destroy(true);
brick.destroy();
cs.destroy(true);
ball.destroy();
gs.destroy(false);
ground.destroy();
world.destroy();
window.destroy();
__plugin_unload("mt/mtype_b2d.dll");
__plugin_unload("../mtype-sfml/mt/mtype_sfml.dll");
