#include <irrlicht.h>
#include <iostream>
#include <cmath>
#include <vector>

#include "Application.h"
#include "JuiceBoxEventListener.h"
#include "ImGuiInputHandler.h"
#include "editor/Editor.h"

// Helpers
#include "helpers/Mesh.h"

// ImGui includes
#include "imgui.h"
#include "imgui_impl_opengl3.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#ifdef _IRR_WINDOWS_
#pragma comment(lib, "Irrlicht.lib")
#endif

// Define the bridge function AFTER the class definition
void forwardEventToImGui(const SEvent& event) {
    // Safety check: Only forward events if the ImGui context has been created
    if (ImGui::GetCurrentContext() != nullptr) {
        ImGuiInputHandler::processEvent(event);
    }
}
bool ImGuiInputHandler::wantCaptureMouse = false;
bool ImGuiInputHandler::wantCaptureKeyboard = false;

int main() {
    /* ================================
    SETUP
    =================================*/
    Application app;
    app.BeginCore();
    app.BeginGUI();

    Editor editor(app);

    /* ================================
    MAIN LOOP 
    =================================*/
    while(app.device->run()) {
        if (app.receiver.IsKeyDown(KEY_ESCAPE)) {
            app.device->closeDevice(); 
        }

        if (app.receiver.IsKeyDown(KEY_KEY_A)) {
            editor.ClearVertices();
        }

        if (app.receiver.IsKeyDown(KEY_KEY_Q)) {
            editor.ChangeMode(EditorMode::VERTEX);
        }
        
        if (app.receiver.IsKeyDown(KEY_KEY_W)) {
            editor.ChangeMode(EditorMode::EDGE);
        }

        if (app.receiver.IsKeyDown(KEY_KEY_E)) {
            editor.ChangeMode(EditorMode::FACE);
        }

        if (app.device->isWindowActive()) {
            /* ================================
            USER INTERACTION
            =================================*/


            /* ================================
            UPDATE
            =================================*/
            editor.Update();

            /* ================================
            RENDER
            =================================*/
            app.driver->beginScene(true, true, SColor(255, 40, 40, 40));
            editor.Draw();
            app.driver->endScene();
            app.receiver.UpdateLastPosition();
            app.receiver.EndFrame();
        }
    }

    return 0;
}

// --- Main Loop ---

// int main() {
//     Application app;
//     app.BeginCore();
//     app.BeginGUI();

//     Editor editor(app);

//     ISceneCollisionManager* coll = app.smgr->getSceneCollisionManager();

//     // Scene Setup
//     IMeshSceneNode* testMesh = app.smgr->addCubeSceneNode(10.0f);
//     if (testMesh) {
//         testMesh->setPosition(vector3df(0, 0, 0));
//         testMesh->setMaterialFlag(EMF_LIGHTING, false);
//     }

//     app.smgr->addLightSceneNode(0, vector3df(0, 20, -20), SColorf(1.0f, 1.0f, 1.0f), 20.0f);

//     // Cameras
//     ICameraSceneNode* camTop = app.smgr->addCameraSceneNode(0, vector3df(0, 50, 0), vector3df(0, 0, 0));
//     camTop->setUpVector(vector3df(0, 0, 1)); 
    
//     ICameraSceneNode* camModel = app.smgr->addCameraSceneNode(0, vector3df(10, 10, -10), vector3df(0, 0, 0));
//     ICameraSceneNode* camFront = app.smgr->addCameraSceneNode(0, vector3df(0, 0, -50), vector3df(0, 0, 0));
//     ICameraSceneNode* camRight = app.smgr->addCameraSceneNode(0, vector3df(50, 0, 0), vector3df(0, 0, 0));

//     matrix4 ortho;
//     ortho.buildProjectionMatrixOrthoLH(30, 22, 0, 100); 

//     camTop->setProjectionMatrix(ortho, true);
//     camFront->setProjectionMatrix(ortho, true);
//     camRight->setProjectionMatrix(ortho, true);

//     f32 cameraRadius = 20.0f;
//     f32 theta = 0.0f;
//     f32 phi = 45.0f;
    
//     position2di lastMousePos = app.receiver.MouseState.Position;

//     ISceneNode* currentMarker = 0;
//     VertexSelection selection;
//     bool isDragging = false;
//     int activeViewportType = -1;
    
//     bool showTestWindow = true;
//     int clickCount = 0;
//     float cubeScale = 1.0f;

//     while(app.device->run()) {
        
//         if (app.receiver.IsKeyDown(KEY_ESCAPE)) {
//             app.device->closeDevice(); 
//         }

//         if (app.device->isWindowActive()) {
//             dimension2d<u32> screenSize = app.driver->getScreenSize();
//             s32 w = screenSize.Width;
//             s32 h = screenSize.Height;
//             s32 midW = w / 2;
//             s32 midH = h / 2;

//             rect<s32> topLeftRect(0, 0, midW, midH);
//             rect<s32> topRightRect(midW, 0, w, midH);
//             rect<s32> bottomLeftRect(0, midH, midW, h);
//             rect<s32> bottomRightRect(midW, midH, w, h);

//             ICameraSceneNode* activeCam = 0;
//             rect<s32> activeRect;
//             bool inOrthoViewport = false;

//             if (isDragging) {
//                 if (activeViewportType == 0) { 
//                     activeCam = camTop; activeRect = topLeftRect; inOrthoViewport = true; 
//                 } else if (activeViewportType == 1) { 
//                     activeCam = camFront; activeRect = bottomLeftRect; inOrthoViewport = true; 
//                 } else if (activeViewportType == 2) { 
//                     activeCam = camRight; activeRect = bottomRightRect; inOrthoViewport = true; 
//                 }
//             } else {
//                 position2di mPos = app.receiver.MouseState.Position;
//                 if (topLeftRect.isPointInside(mPos)) {
//                     activeCam = camTop; activeRect = topLeftRect; inOrthoViewport = true; activeViewportType = 0;
//                 }
//                 else if (bottomLeftRect.isPointInside(mPos)) {
//                     activeCam = camFront; activeRect = bottomLeftRect; inOrthoViewport = true; activeViewportType = 1;
//                 }
//                 else if (bottomRightRect.isPointInside(mPos)) {
//                     activeCam = camRight; activeRect = bottomRightRect; inOrthoViewport = true; activeViewportType = 2;
//                 }
//                 else if (topRightRect.isPointInside(mPos)) {
//                     activeViewportType = 3;
//                 }
//                 else {
//                     activeViewportType = -1; 
//                 }
//             }

//             if (!ImGuiInputHandler::wantCaptureMouse && app.receiver.MouseState.LeftButtonDown) {
                
//                 if (!isDragging && activeViewportType == 3) {
//                     s32 dx = app.receiver.MouseState.Position.X - lastMousePos.X;
//                     s32 dy = app.receiver.MouseState.Position.Y - lastMousePos.Y;
//                     theta -= dx * 0.2f; 
//                     phi += dy * 0.2f;  
//                     if (phi > 89.0f) phi = 89.0f;
//                     if (phi < -89.0f) phi = -89.0f;
//                     f32 r = cameraRadius;
//                     f32 radTheta = theta * DEGTORAD;
//                     f32 radPhi = phi * DEGTORAD;
//                     f32 x = r * cos(radPhi) * sin(radTheta);
//                     f32 y = r * sin(radPhi);
//                     f32 z = r * cos(radPhi) * cos(radTheta) * -1; 
//                     camModel->setPosition(vector3df(x, y, z));
//                     camModel->setTarget(vector3df(0,0,0)); 
//                 }

//                 if (inOrthoViewport && activeCam && testMesh) {
                    
//                     if (!isDragging && !selection.isSelected) {
//                         vector3df hitPos;
//                         u32 bufIdx;
//                         std::vector<u32> indices;
//                         f32 selectThreshold = 45.0f; 

//                         bool found = VertexSelector::Get(app.smgr, app.driver, 
//                                                     testMesh, activeCam, 
//                                                     activeRect, app.receiver.MouseState.Position,
//                                                     selectThreshold, hitPos, bufIdx, indices);

//                         if (found) {
//                             selection.isSelected = true;
//                             selection.bufferIndex = bufIdx;
//                             selection.vertexIndices = indices;
//                             selection.worldPos = hitPos;
//                             isDragging = true;

//                             if (currentMarker) currentMarker->remove();
//                             currentMarker = app.smgr->addSphereSceneNode(0.5f);
//                             currentMarker->setPosition(hitPos);
//                             currentMarker->setMaterialFlag(EMF_LIGHTING, false);
//                             currentMarker->setMaterialFlag(EMF_ZBUFFER, false);
//                         }
//                     }
//                     else if (isDragging && selection.isSelected) {
//                         vector3df newPos = getDragPosition(coll, activeCam, 
//                                                           app.receiver.MouseState.Position, 
//                                                           selection.worldPos, activeViewportType,
//                                                           activeRect);

//                         Mesh::Update(testMesh, selection.bufferIndex, selection.vertexIndices, newPos);
                        
//                         selection.worldPos = newPos;
//                         if (currentMarker) currentMarker->setPosition(newPos);
//                     }
//                 }
//             }
//             else if (!app.receiver.MouseState.LeftButtonDown) {
//                 isDragging = false;
//                 selection.isSelected = false;
//             }
            
//             lastMousePos = app.receiver.MouseState.Position;

//             // Send screen dimensions
//             ImGuiIO& io = ImGui::GetIO();
//             io.DisplaySize = ImVec2((float)screenSize.Width, (float)screenSize.Height);

//             ImGui_ImplOpenGL3_NewFrame();
//             ImGui::NewFrame();

//             // 2. Render the Toolbar with fixed 5% height
//             float toolbarHeight = screenSize.Height * 0.05f; // 5% of window height

//             ImGui::SetNextWindowPos(ImVec2(0, 0));
//             ImGui::SetNextWindowSize(ImVec2((float)screenSize.Width, toolbarHeight));

//             ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | 
//                                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | 
//                                             ImGuiWindowFlags_MenuBar;

//             if (ImGui::Begin("MainToolbar", nullptr, window_flags)) {
//                 if (ImGui::BeginMenuBar()) {
//                     if (ImGui::BeginMenu("File")) {
//                         if (ImGui::MenuItem("Exit")) app.device->closeDevice();
//                         ImGui::EndMenu();
//                     }
//                     // ... other menus
//                     ImGui::EndMenuBar();
//                 }
//             }
//             ImGui::End();

//             ImGui::Render();

//             app.driver->beginScene(true, true, SColor(255, 40, 40, 40));

//             // Viewport Rendering
//             app.driver->setViewPort(rect<s32>(0, 0, midW, midH));
//             app.smgr->setActiveCamera(camTop);    
//             if(testMesh) testMesh->setMaterialFlag(EMF_WIREFRAME, true);
//             if(testMesh) testMesh->setMaterialFlag(EMF_LIGHTING, false);
//             app.smgr->drawAll();
            
//             app.driver->setViewPort(rect<s32>(midW, 0, w, midH));
//             app.smgr->setActiveCamera(camModel);
//             if(testMesh) testMesh->setMaterialFlag(EMF_WIREFRAME, false);
//             if(testMesh) testMesh->setMaterialFlag(EMF_LIGHTING, true);
//             app.smgr->drawAll();

//             app.driver->setViewPort(rect<s32>(0, midH, midW, h));
//             app.smgr->setActiveCamera(camFront);
//             if(testMesh) testMesh->setMaterialFlag(EMF_WIREFRAME, true);
//             if(testMesh) testMesh->setMaterialFlag(EMF_LIGHTING, false);
//             app.smgr->drawAll();

//             app.driver->setViewPort(rect<s32>(midW, midH, w, h));
//             app.smgr->setActiveCamera(camRight);
//             if(testMesh) testMesh->setMaterialFlag(EMF_WIREFRAME, true);
//             if(testMesh) testMesh->setMaterialFlag(EMF_LIGHTING, false);
//             app.smgr->drawAll();

//             app.driver->setViewPort(rect<s32>(0, 0, w, h));
//             ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            
//             SColor borderColor(255, 100, 100, 100);
//             app.driver->draw2DLine(position2d<s32>(midW, 0), position2d<s32>(midW, h), borderColor);
//             app.driver->draw2DLine(position2d<s32>(0, midH), position2d<s32>(w, midH), borderColor);

//             app.driver->endScene();
//         }
//     }

//     return 0;
// }