#pragma once

#include "irrlicht.h"
#include "Application.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class Camera {
public:
    Camera(
        Application& application,
        vector3df position,
        vector3df lookat,
        bool isOrtho
    );
    ~Camera();

    ICameraSceneNode* GetCameraSceneNode() { return _camera; }
    void SetUpVector(vector3df up) { _camera->setUpVector(up); }
    void Rotate(float mouseDeltaX, float mouseDeltaY);

private:
    ICameraSceneNode* _camera;

    vector3df _position;
    vector3df _lookat;
    f32 _cameraRadius = 20.0f;
    f32 _theta = 0.0f;
    f32 _phi = 45.0f;
    float _sensitivity = 0.4f;

    matrix4 _orthographic;
    
    // Orthographic projection constants
    static constexpr int ORTHO_WIDTH_VIEW = 30;  
    static constexpr int ORTHO_HEIGHT_VIEW = 22; // Need to update this to equal the viewport ratio
    static constexpr int ORTHO_NEAR = 0;
    static constexpr int ORTHO_FAR = 100;

    void _setInitialPosition();
};