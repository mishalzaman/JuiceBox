#include <irrlicht.h>
#include <iostream>
#include <cmath>

#include "Core.h"
#include "JuiceBoxEventListener.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#ifdef _IRR_WINDOWS_
#pragma comment(lib, "Irrlicht.lib")
#endif

int main() {
    Core engine;

    // --- Scene Setup ---
    IMeshSceneNode* cube = engine.smgr->addCubeSceneNode(10.0f);
    if (cube) {
        cube->setPosition(vector3df(0, 0, 0));
        cube->setMaterialFlag(EMF_LIGHTING, false); 
    }

    engine.smgr->addLightSceneNode(0, vector3df(0, 20, -20), SColorf(1.0f, 1.0f, 1.0f), 20.0f);

    // --- Cameras ---
    // 1. Top Left (Top View)
    ICameraSceneNode* camTop = engine.smgr->addCameraSceneNode(0, vector3df(0, 50, 0), vector3df(0, 0, 0));
    camTop->setUpVector(vector3df(0, 0, 1)); 
    
    // 2. Top Right (Model View) 
    ICameraSceneNode* camModel = engine.smgr->addCameraSceneNode(0, vector3df(10, 10, -10), vector3df(0, 0, 0));

    // 3. Bottom Left (Front View)
    ICameraSceneNode* camFront = engine.smgr->addCameraSceneNode(0, vector3df(0, 0, -50), vector3df(0, 0, 0));
    
    // 4. Bottom Right (Right View)
    ICameraSceneNode* camRight = engine.smgr->addCameraSceneNode(0, vector3df(50, 0, 0), vector3df(0, 0, 0));

    // Ortho Matrix for wireframes
    matrix4 ortho;
    ortho.buildProjectionMatrixOrthoLH(30, 22, 0, 100); 

    camTop->setProjectionMatrix(ortho, true);
    camFront->setProjectionMatrix(ortho, true);
    camRight->setProjectionMatrix(ortho, true);

    // --- Orbit Control Variables ---
    f32 cameraRadius = 20.0f;
    f32 theta = 0.0f; // Horizontal angle
    f32 phi = 45.0f;  // Vertical angle
    
    position2di lastMousePos = engine.receiver.MouseState.Position;

    while(engine.device->run()) {
        
        if (engine.receiver.IsKeyDown(KEY_ESCAPE)) {
            engine.device->closeDevice(); 
        }

        if (engine.device->isWindowActive()) {
            const int PANEL_HEIGHT = 32;
            const int PANEL_WIDTH = 32;
            dimension2d<u32> screenSize = engine.driver->getScreenSize();
            s32 w = screenSize.Width;
            s32 h = screenSize.Height;
            s32 midW = w / 2;
            s32 midH = h / 2;

            // Rectangles of each viewport
            rect<s32> topLeftRect(0, 0, midW, midH);
            rect<s32> topRightRect(midW, 0, w, midH);
            rect<s32> bottomLeftRect(0, midH, midW, h);
            rect<s32> bottomRightRect(midW, midH, w, h);

            /*==========================================
            CAMERA MODEL ROTATION
            ===========================================*/
            if (engine.receiver.MouseState.LeftButtonDown) {
                if (topLeftRect.isPointInside(engine.receiver.MouseState.Position)) {
                    std::cout << "Top left" << std::endl;
                }
                if (bottomLeftRect.isPointInside(engine.receiver.MouseState.Position)) {
                    std::cout << "Bottom left" << std::endl;
                }
                if (bottomRightRect.isPointInside(engine.receiver.MouseState.Position)) {
                    std::cout << "Bottom right" << std::endl;
                }

                if (topRightRect.isPointInside(engine.receiver.MouseState.Position)) {
                    
                    s32 dx = engine.receiver.MouseState.Position.X - lastMousePos.X;
                    s32 dy = engine.receiver.MouseState.Position.Y - lastMousePos.Y;

                    // Apply Sensitivity
                    theta -= dx * 0.2f; 
                    
                    // --- CHANGED: INVERTED VERTICAL LOOK ---
                    // Changed from "-=" to "+="
                    phi += dy * 0.2f;  

                    if (phi > 89.0f) phi = 89.0f;
                    if (phi < -89.0f) phi = -89.0f;

                    // Orbit Math
                    f32 r = cameraRadius;
                    f32 radTheta = theta * DEGTORAD;
                    f32 radPhi = phi * DEGTORAD;

                    f32 x = r * cos(radPhi) * sin(radTheta);
                    f32 y = r * sin(radPhi);
                    f32 z = r * cos(radPhi) * cos(radTheta) * -1; 

                    camModel->setPosition(vector3df(x, y, z));
                    camModel->setTarget(vector3df(0,0,0)); 
                }
            }
            
            lastMousePos = engine.receiver.MouseState.Position;

            engine.driver->beginScene(true, true, SColor(255, 40, 40, 40));

            /*==========================================
            VIEWPORTS
            ===========================================*/
            // TOP LEFT
            engine.driver->setViewPort(rect<s32>(0, 0, midW, midH));
            engine.smgr->setActiveCamera(camTop);    
            if(cube) {
                cube->setMaterialFlag(EMF_WIREFRAME, true);
                cube->setMaterialFlag(EMF_LIGHTING, false);
            }
            engine.smgr->drawAll();
            
            // TOP RIGHT
            engine.driver->setViewPort(rect<s32>(midW, 0, w, midH));
            engine.smgr->setActiveCamera(camModel);
            if(cube) {
                cube->setMaterialFlag(EMF_WIREFRAME, false);
                cube->setMaterialFlag(EMF_LIGHTING, true);
            }
            engine.smgr->drawAll();

            // BOTTOM LEFT
            engine.driver->setViewPort(rect<s32>(0, midH, midW, h));
            engine.smgr->setActiveCamera(camFront);
            if(cube) {
                cube->setMaterialFlag(EMF_WIREFRAME, true);
                cube->setMaterialFlag(EMF_LIGHTING, false);
            }
            engine.smgr->drawAll();

            // BOTTOM RIGHT
            engine.driver->setViewPort(rect<s32>(midW, midH, w, h));
            engine.smgr->setActiveCamera(camRight);
            if(cube) {
                cube->setMaterialFlag(EMF_WIREFRAME, true);
                cube->setMaterialFlag(EMF_LIGHTING, false);
            }
            engine.smgr->drawAll();

            /*==========================================
            BORDERS
            ===========================================*/
            engine.driver->setViewPort(rect<s32>(0, 0, w, h));
            SColor borderColor(255, 100, 100, 100);
            engine.driver->draw2DLine(position2d<s32>(midW, 0), position2d<s32>(midW, h), borderColor);
            engine.driver->draw2DLine(position2d<s32>(0, midH), position2d<s32>(w, midH), borderColor);

            engine.driver->endScene();
        }
    }

    return 0;
}