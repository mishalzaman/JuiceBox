#pragma once

#include "Application.h"

class Editor {
public:
    Editor(Application& application);  // Pass by reference
    ~Editor();

    void Draw();
    void Update();
    
private:
    Application& _application;  // Store as reference

    void _setupDefaultMesh();
    void _setupCameras();

    // Cameras
    ICameraSceneNode* _camTop;
    ICameraSceneNode* _camModel;
    ICameraSceneNode* _camFront;
    ICameraSceneNode* _camRight;
};