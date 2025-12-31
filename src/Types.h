#pragma once

#include <irrlicht.h>
#include <vector>

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

struct VertexSelection {
    bool isSelected = false;
    u32 bufferIndex = 0;
    std::vector<u32> vertexIndices;
    vector3df worldPos;
};

struct EdgeSelection {
    bool isSelected = false;
    u32 bufferIndex = 0;
    u32 vertexIndex1 = 0;
    u32 vertexIndex2 = 0;
    vector3df worldPos1;
    vector3df worldPos2;
};

struct FaceSelection {
    bool isSelected = false;
    u32 bufferIndex = 0;
    u32 vertexIndex1 = 0;
    u32 vertexIndex2 = 0;
    u32 vertexIndex3 = 0;
    vector3df worldPos1;
    vector3df worldPos2;
    vector3df worldPos3;
};