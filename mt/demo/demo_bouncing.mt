// Cross-plugin demo: physics from mtype-box2d, rendering from mtype-sfml.
//
// 10 dynamic boxes drop onto a horizontal floor. Each frame: step the
// world, then render each body as a SFML RectangleShape positioned and
// rotated from body.position() / body.rotation().
//
// Requires mtype_sfml.dll alongside mtype_b2d.dll. The SFML wrappers are
// imported from a sibling repo at ../../mtype-sfml/mt/lib/.

import * from "../lib/Box2D.mt";
import * from "../lib/Body.mt";
import * from "../lib/Shape.mt";

import * from "../../mtype-sfml/mt/lib/Sfml.mt";
import * from "../../mtype-sfml/mt/lib/Graphics.mt";
import * from "../../mtype-sfml/mt/lib/System.mt";

__plugin_load("mt/mtype_b2d.dll");
__plugin_load("../mtype-sfml/mt/mtype_sfml.dll");

RenderWindow window = Sfml::createWindow("mtype-box2d bouncing boxes", 800, 600);

// World: positive gravity-y pulls bodies toward +Y. We map +Y in physics
// to +Y on screen below by adding to the screen origin (no flip needed for
// "boxes fall down" semantics — Box2D doesn't pick an axis convention).
World world = Box2D::createWorld(0.0, 9.8);

// Ground: a static body at y=5 with a horizontal segment from (-10, 0) to
// (10, 0) in body-local space.
BodyDef groundDef = new BodyDef();
groundDef.setType(BodyType::staticBody());
groundDef.setPosition(0.0, 5.0);
Body ground = Bodies::create(world, groundDef);
groundDef.destroy();

ShapeDef groundShapeDef = new ShapeDef();
groundShapeDef.setFriction(0.6);
Shape groundShape = Shapes::createSegment(ground, groundShapeDef,
                                            -10.0, 0.0, 10.0, 0.0);
groundShapeDef.destroy();

// Stack of 10 dynamic boxes, each 1m x 1m, falling from above.
int N = 10;
Body[] boxes = new Body[N];
int i = 0;
while (i < N) {
    BodyDef bd = new BodyDef();
    bd.setType(BodyType::dynamicBody());
    // Stagger horizontally a bit so they don't overlap exactly.
    bd.setPosition(0.0 + (i - 5) * 0.05, -2.0 - i * 1.2);
    bd.setAngle(Box2D::radians(i * 7.0));
    Body b = Bodies::create(world, bd);
    bd.destroy();

    ShapeDef sd = new ShapeDef();
    sd.setDensity(1.0);
    sd.setFriction(0.3);
    sd.setRestitution(0.2);
    Shape s = Shapes::createBox(b, sd, 0.5, 0.5);
    sd.destroy();

    boxes[i] = b;
    i = i + 1;
}

// Rendering. 30 px per metre, world origin at screen (400, 100).
float scale = 30.0;
float originX = 400.0;
float originY = 100.0;

RectangleShape rect = Rectangles::create(scale, scale);
rect.setFillColor(220, 80, 60, 255);
rect.setOutlineColor(255, 255, 255, 255);
rect.setOutlineThickness(1.0);
rect.setOrigin(scale * 0.5, scale * 0.5);

int evClosed     = Sfml::closedEventId();
int evKeyPressed = Sfml::keyPressedEventId();
int keyEsc = Key::escape();

Clock frame = Clocks::create();

while (window.isOpen()) {
    int ev = window.pollEvent();
    while (ev != 0) {
        if (ev == evClosed) {
            window.close();
        } else if (ev == evKeyPressed) {
            if (Event::key() == keyEsc) { window.close(); }
        }
        ev = window.pollEvent();
    }

    float dt = frame.restartSeconds();
    if (dt > 0.05) { dt = 0.05; }
    world.step(dt, 4);

    window.clear(20, 22, 30, 255);

    // Draw ground as a long thin rectangle. Segment from (-10,0) to (10,0)
    // in body-local space, body sits at (0, 5).
    RectangleShape floor = Rectangles::create(20.0 * scale, 4.0);
    floor.setFillColor(180, 180, 180, 255);
    floor.setOrigin(10.0 * scale, 2.0);
    floor.setPosition(originX, originY + 5.0 * scale);
    Draw::rect(window, floor);
    floor.destroy();

    // Draw each box at its body's current transform.
    int j = 0;
    while (j < N) {
        float[] p = boxes[j].position();
        float angle = boxes[j].rotation();
        rect.setPosition(originX + p[0] * scale, originY + p[1] * scale);
        rect.setRotation(Box2D::degrees(angle));
        Draw::rect(window, rect);
        j = j + 1;
    }

    window.display();
}

// Cleanup.
frame.destroy();
rect.destroy();
int k = 0;
while (k < N) { boxes[k].destroy(); k = k + 1; }
groundShape.destroy(false);
ground.destroy();
world.destroy();
window.destroy();
__plugin_unload("mt/mtype_b2d.dll");
__plugin_unload("../mtype-sfml/mt/mtype_sfml.dll");
