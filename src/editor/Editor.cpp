#include "Editor.h"
#include <iostream>

Editor::Editor(Application& application):
    _application(application)
{
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

void Editor::_setupCameras()
{
    _camTop = _application.smgr->addCameraSceneNode(0, vector3df(0, 50, 0), vector3df(0, 0, 0));
    _camTop->setUpVector(vector3df(0, 0, 1)); 
    
    _camModel = _application.smgr->addCameraSceneNode(0, vector3df(10, 10, -10), vector3df(0, 0, 0));
    _camFront = _application.smgr->addCameraSceneNode(0, vector3df(0, 0, -50), vector3df(0, 0, 0));
    _camRight = _application.smgr->addCameraSceneNode(0, vector3df(50, 0, 0), vector3df(0, 0, 0));

}
