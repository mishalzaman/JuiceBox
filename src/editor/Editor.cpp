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
      _cameraRight(application, CAMERA_RIGHT_POS, CAMERA_LOOKAT, true),
      _vTop(_application, _cameraTop),
      _vModel(_application, _cameraModel),
      _vFront(_application, _cameraFront),
      _vRight(_application, _cameraRight)
{
    // Set the custom up vector for the top camera
    _cameraTop.SetUpVector(CAMERA_TOP_UP);

    _setViewports();
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

void Editor::_setViewports()
{
    dimension2d<u32> screenSize = _application.driver->getScreenSize();
    s32 w = screenSize.Width;
    s32 h = screenSize.Height;
    s32 midW = w / 2;
    s32 midH = h / 2;

    _vTop.UpdateViewport(0, 0, midW, midH);
    _vModel.UpdateViewport(midW, 0, w, midH);
    _vFront.UpdateViewport(0, midH, midW, h);
    _vRight.UpdateViewport(midW, midH, w, h);
}