#include "Editor.h"
#include <iostream>

const vector3df Editor::CAMERA_LOOKAT = vector3df(0, 0, 0);
const vector3df Editor::CAMERA_TOP_POS = vector3df(0, 50, 0);
const vector3df Editor::CAMERA_TOP_UP = vector3df(0, 0, 1);
const vector3df Editor::CAMERA_MODEL_POS = vector3df(10, 10, -10);
const vector3df Editor::CAMERA_FRONT_POS = vector3df(0, 0, -50);
const vector3df Editor::CAMERA_RIGHT_POS = vector3df(50, 0, 0);

Editor::Editor(Application& application)
    : _application(application),
      _cameraTop(application, CAMERA_TOP_POS, CAMERA_LOOKAT, true),
      _cameraModel(application, CAMERA_MODEL_POS, CAMERA_LOOKAT, false),
      _cameraFront(application, CAMERA_FRONT_POS, CAMERA_LOOKAT, true),
      _cameraRight(application, CAMERA_RIGHT_POS, CAMERA_LOOKAT, true)
{
    // Set the custom up vector for the top camera
    _cameraTop.SetUpVector(CAMERA_TOP_UP);
}

Editor::~Editor()
{
    std::cout << "Shutdown Editor" << std::endl;
}

void Editor::Draw()
{
}

void Editor::Update()
{
}

void Editor::_setupDefaultMesh()
{
    ISceneCollisionManager* coll = _application.smgr->getSceneCollisionManager();

    // Scene Setup
    IMeshSceneNode* testMesh = _application.smgr->addCubeSceneNode(10.0f);
    if (testMesh) {
        testMesh->setPosition(vector3df(0, 0, 0));
        testMesh->setMaterialFlag(EMF_LIGHTING, false);
    }

    _application.smgr->addLightSceneNode(0, vector3df(0, 20, -20), SColorf(1.0f, 1.0f, 1.0f), 20.0f);
}

