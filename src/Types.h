#pragma once

#include <irrlicht.h>

enum EditorMode : int {
    VERTEX = 0,
    EDGE = 1,
    FACE = 2
};

enum ViewportType : int {
    TOP = 0,
    BOTTOM = 1,
    FRONT = 2,
    BACK = 3,
    RIGHT = 4,
    LEFT = 5,
    MODEL = 6
};

static const vector3df ViewportCameraNormals[] = {
    vector3df(0, 1, 0),  // TOP (index 0)
    vector3df(0, -1, 0),  // BOTTOM (index 0)
    vector3df(0, 0, 1),  // FRONT (index 1)
    vector3df(0, 0, -1),  // BACK (index 1)
    vector3df(-1, 0, 0),  // RIGHT (index 2)
    vector3df(1, 0, 0),  // LEFT (index 2)
    vector3df(1, 1, 1)  // MODEL (index 2)
};