#pragma once

#include "Application.h"
#include "Camera.h"
#include "Viewport.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class Editor {
public:
    Editor(Application& application);
    ~Editor();

    void Draw();
    void Update();
    
private:
    Application& _application;

    void _setupDefaultMesh();
    void _setViewports();

    // Camera constants - declare only
    static const vector3df CAMERA_LOOKAT;
    static const vector3df CAMERA_TOP_POS;
    static const vector3df CAMERA_TOP_UP;
    static const vector3df CAMERA_MODEL_POS;
    static const vector3df CAMERA_FRONT_POS;
    static const vector3df CAMERA_RIGHT_POS;

    Camera _cameraTop;
    Camera _cameraModel;
    Camera _cameraFront;
    Camera _cameraRight;
    Viewport _vTop;
    Viewport _vModel;
    Viewport _vFront;
    Viewport _vRight;
};