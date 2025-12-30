#pragma once

#include <vector>
#include <memory>

#include "Application.h"
#include "Camera.h"
#include "Viewport.h"
#include "Model.h"
#include "Types.h"
#include "utility/UVertex.h"
#include "helpers/Mesh.h"

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

    void ClearVertices();
    void ChangeMode(EditorMode mode) { _mode = mode; }
    
private:
    Application& _application;
    dimension2d<u32> _screenSize;
    ISceneCollisionManager* _collisionManager;
    IMeshSceneNode* _defaultMesh;


    void _setupDefaultMesh();
    void _setViewports();
    void _setActiveViewport();

    // Camera constants
    static const vector3df CAMERA_LOOKAT;
    static const vector3df CAMERA_TOP_POS;
    static const vector3df CAMERA_TOP_UP;
    static const vector3df CAMERA_MODEL_POS;
    static const vector3df CAMERA_FRONT_POS;
    static const vector3df CAMERA_RIGHT_POS;

    // Camera and Viewports
    Camera _cameraTop;
    Camera _cameraModel;
    Camera _cameraFront;
    Camera _cameraRight;
    Viewport _vTop;
    Viewport _vModel;
    Viewport _vFront;
    Viewport _vRight;
    Viewport* _activeViewport;

    // Vertex Selections
    std::unique_ptr<Model> _model;

    EditorMode _mode;
};