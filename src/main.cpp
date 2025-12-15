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

// Returns true if a vertex was found within the threshold
bool getClosestVertex(ISceneManager* smgr, IVideoDriver* driver, 
                      IMeshSceneNode* node, ICameraSceneNode* camera, 
                      rect<s32> viewport, position2di mousePos,
                      f32 pixelThreshold, vector3df& outPos)
{
    bool found = false;
    f32 minDistSq = pixelThreshold * pixelThreshold;
    f32 closestDepth = FLT_MAX; 

    camera->updateAbsolutePosition(); 
    matrix4 view = camera->getViewMatrix();
    matrix4 proj = camera->getProjectionMatrix();
    matrix4 world = node->getAbsoluteTransformation();

    matrix4 viewProj = proj * view;

    f32 vpW = (f32)viewport.getWidth();
    f32 vpH = (f32)viewport.getHeight();
    f32 vpX = (f32)viewport.UpperLeftCorner.X;
    f32 vpY = (f32)viewport.UpperLeftCorner.Y;

    IMesh* mesh = node->getMesh();
    vector3df camPos = camera->getAbsolutePosition();

    for (u32 b = 0; b < mesh->getMeshBufferCount(); ++b) {
        IMeshBuffer* mb = mesh->getMeshBuffer(b);
        S3DVertex* vertices = (S3DVertex*)mb->getVertices();

        for (u32 v = 0; v < mb->getVertexCount(); ++v) {
            vector3df vPos = vertices[v].Pos;
            
            // 2. Transform Local -> World
            world.transformVect(vPos);

            // 3. Transform World -> Clip Space (Manually)
            // --- FIX START ---
            f32 transformedPos[4] = { vPos.X, vPos.Y, vPos.Z, 1.0f }; // Init array
            viewProj.multiplyWith1x4Matrix(transformedPos);           // Transform in-place
            // --- FIX END ---

            if (transformedPos[3] == 0) continue;

            f32 zDiv = 1.0f / transformedPos[3];
            f32 ndcX = transformedPos[0] * zDiv;
            f32 ndcY = transformedPos[1] * zDiv;

            // 4. Map NDC -> Screen Space
            f32 screenX = (ndcX + 1.0f) * 0.5f * vpW + vpX;
            f32 screenY = (1.0f - ndcY) * 0.5f * vpH + vpY;

            f32 dx = screenX - mousePos.X;
            f32 dy = screenY - mousePos.Y;
            f32 distSq = dx*dx + dy*dy;

            if (distSq < minDistSq) {
                f32 trueDepthSq = vPos.getDistanceFromSQ(camPos);

                if (trueDepthSq < closestDepth) {
                    closestDepth = trueDepthSq;
                    outPos = vPos;
                    found = true;
                }
            }
        }
    }

    return found;
}

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

    ISceneNode* currentMarker = 0;

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

            ICameraSceneNode* activeCam = 0;
            rect<s32> activeRect;
            bool clickedViewport = false;

            /*==========================================
            CAMERA MODEL ROTATION
            ===========================================*/
            if (engine.receiver.MouseState.LeftButtonDown) {
                if (topLeftRect.isPointInside(engine.receiver.MouseState.Position)) {
                    std::cout << "Clicked Top Left" << std::endl;
                    activeCam = camTop;
                    activeRect = topLeftRect;
                    clickedViewport = true;
                }
                else if (bottomLeftRect.isPointInside(engine.receiver.MouseState.Position)) {
                    std::cout << "Clicked Bottom Left" << std::endl;
                    activeCam = camFront;
                    activeRect = bottomLeftRect;
                    clickedViewport = true;
                }
                else if (bottomRightRect.isPointInside(engine.receiver.MouseState.Position)) {
                    std::cout << "Clicked Bottom Right" << std::endl;
                    activeCam = camRight;
                    activeRect = bottomRightRect;
                    clickedViewport = true;
                }

                else if (topRightRect.isPointInside(engine.receiver.MouseState.Position)) {
                    
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

                if (clickedViewport && cube) {
                    vector3df hitPos;
                    // Threshold in PIXELS now (e.g., 15px radius)
                    f32 selectThreshold = 45.0f; 

                    bool found = getClosestVertex(engine.smgr, engine.driver, 
                                                cube, activeCam, 
                                                activeRect, engine.receiver.MouseState.Position,
                                                selectThreshold, hitPos);

                    if (found) {
                        std::cout << "Selected Vertex: " << hitPos.X << ", " << hitPos.Y << ", " << hitPos.Z << std::endl;
                        
                        if (currentMarker) {
                            currentMarker->remove();
                            currentMarker = 0;
                        }

                        currentMarker = engine.smgr->addSphereSceneNode(0.5f);
                        currentMarker->setPosition(hitPos);
                        currentMarker->setMaterialFlag(EMF_LIGHTING, false);
                        currentMarker->setMaterialFlag(EMF_ZBUFFER, false); // Draw on top
                    }
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