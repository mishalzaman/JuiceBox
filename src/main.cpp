#include <irrlicht.h>
#include <iostream>
#include <cmath>
#include <vector>

#include "Core.h"
#include "JuiceBoxEventListener.h"

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

// --- Structures and Helper Functions ---

struct VertexSelection {
    bool isSelected = false;
    u32 bufferIndex = 0;
    std::vector<u32> vertexIndices;
    vector3df worldPos;
};

bool getClosestVertex(ISceneManager* smgr, IVideoDriver* driver, 
                      IMeshSceneNode* node, ICameraSceneNode* camera, 
                      rect<s32> viewport, position2di mousePos,
                      f32 pixelThreshold, vector3df& outPos,
                      u32& outBufferIndex, std::vector<u32>& outIndices)
{
    // ... (Keep existing implementation of getClosestVertex)
    bool found = false;
    f32 minDistSq = pixelThreshold * pixelThreshold;
    f32 closestDepth = FLT_MAX; 
    outIndices.clear();

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

    u32 closestVIndex = 0;
    u32 closestBIndex = 0;
    vector3df closestWorldPos;

    for (u32 b = 0; b < mesh->getMeshBufferCount(); ++b) {
        IMeshBuffer* mb = mesh->getMeshBuffer(b);
        S3DVertex* vertices = (S3DVertex*)mb->getVertices();

        for (u32 v = 0; v < mb->getVertexCount(); ++v) {
            vector3df vPos = vertices[v].Pos;
            world.transformVect(vPos);

            f32 transformedPos[4] = { vPos.X, vPos.Y, vPos.Z, 1.0f };
            viewProj.multiplyWith1x4Matrix(transformedPos);

            if (transformedPos[3] == 0) continue;

            f32 zDiv = 1.0f / transformedPos[3];
            f32 ndcX = transformedPos[0] * zDiv;
            f32 ndcY = transformedPos[1] * zDiv;

            f32 screenX = (ndcX + 1.0f) * 0.5f * vpW + vpX;
            f32 screenY = (1.0f - ndcY) * 0.5f * vpH + vpY;

            f32 dx = screenX - mousePos.X;
            f32 dy = screenY - mousePos.Y;
            f32 distSq = dx*dx + dy*dy;

            if (distSq < minDistSq) {
                f32 trueDepthSq = vPos.getDistanceFromSQ(camPos);
                if (trueDepthSq < closestDepth) {
                    closestDepth = trueDepthSq;
                    closestWorldPos = vPos;
                    closestBIndex = b;
                    closestVIndex = v;
                    found = true;
                }
            }
        }
    }

    if (found) {
        outPos = closestWorldPos;
        outBufferIndex = closestBIndex;
        
        IMeshBuffer* mb = mesh->getMeshBuffer(closestBIndex);
        S3DVertex* vertices = (S3DVertex*)mb->getVertices();
        
        matrix4 invWorld;
        node->getAbsoluteTransformation().getInverse(invWorld);
        vector3df targetLocalPos = closestWorldPos;
        invWorld.transformVect(targetLocalPos);

        for (u32 v = 0; v < mb->getVertexCount(); ++v) {
            if (vertices[v].Pos.equals(targetLocalPos, 0.001f)) {
                outIndices.push_back(v);
            }
        }
    }

    return found;
}

vector3df getDragPosition(ISceneCollisionManager* coll, ICameraSceneNode* camera, 
                          position2di mousePos, vector3df originalPos, int viewportType,
                          rect<s32> viewport)
{
    // ... (Keep existing implementation of getDragPosition)
    plane3df dragPlane;

    switch(viewportType) {
        case 0:
            dragPlane = plane3df(originalPos, vector3df(0, 1, 0));
            break;
        case 1:
            dragPlane = plane3df(originalPos, vector3df(0, 0, 1));
            break;
        case 2:
            dragPlane = plane3df(originalPos, vector3df(-1, 0, 0));
            break;
        default:
            return originalPos;
    }

    position2di viewportMouse(
        mousePos.X - viewport.UpperLeftCorner.X,
        mousePos.Y - viewport.UpperLeftCorner.Y
    );
    
    camera->updateAbsolutePosition();
    
    matrix4 viewMat = camera->getViewMatrix();
    matrix4 projMat = camera->getProjectionMatrix();
    matrix4 viewProj = projMat * viewMat;
    matrix4 invViewProj;
    viewProj.getInverse(invViewProj);
    
    f32 vpW = (f32)viewport.getWidth();
    f32 vpH = (f32)viewport.getHeight();
    f32 ndcX = (viewportMouse.X / vpW) * 2.0f - 1.0f;
    f32 ndcY = 1.0f - (viewportMouse.Y / vpH) * 2.0f;
    
    vector3df nearPoint(ndcX, ndcY, 0.0f);
    vector3df farPoint(ndcX, ndcY, 1.0f);
    
    invViewProj.transformVect(nearPoint);
    invViewProj.transformVect(farPoint);
    
    vector3df rayDir = farPoint - nearPoint;
    rayDir.normalize();

    vector3df intersection;
    if (dragPlane.getIntersectionWithLine(nearPoint, rayDir, intersection)) {
        return intersection;
    }

    return originalPos;
}

void updateVertexPositions(IMeshSceneNode* node, u32 bufferIndex, const std::vector<u32>& indices, vector3df worldPos)
{
    // ... (Keep existing implementation of updateVertexPositions)
    IMesh* mesh = node->getMesh();
    if (bufferIndex >= mesh->getMeshBufferCount()) return;
    
    IMeshBuffer* mb = mesh->getMeshBuffer(bufferIndex);
    S3DVertex* vertices = (S3DVertex*)mb->getVertices();
    
    matrix4 worldTransform = node->getAbsoluteTransformation();
    matrix4 invWorld;
    worldTransform.getInverse(invWorld);
    
    vector3df localPos = worldPos;
    invWorld.transformVect(localPos);
    
    for (u32 idx : indices) {
        if (idx < mb->getVertexCount()) {
            vertices[idx].Pos = localPos;
        }
    }
    
    mb->setDirty(EBT_VERTEX);
    mesh->setDirty();
}

// --- ImGui Integration ---

class ImGuiInputHandler {
public:
    static bool wantCaptureMouse;
    static bool wantCaptureKeyboard;
    
    static void processEvent(const SEvent& event) {
        ImGuiIO& io = ImGui::GetIO();
        
        if (event.EventType == EET_MOUSE_INPUT_EVENT) {
            io.MousePos = ImVec2((float)event.MouseInput.X, (float)event.MouseInput.Y);
            
            if (event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN)
                io.MouseDown[0] = true;
            if (event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP)
                io.MouseDown[0] = false;
            if (event.MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN)
                io.MouseDown[1] = true;
            if (event.MouseInput.Event == EMIE_RMOUSE_LEFT_UP)
                io.MouseDown[1] = false;
            if (event.MouseInput.Event == EMIE_MMOUSE_PRESSED_DOWN)
                io.MouseDown[2] = true;
            if (event.MouseInput.Event == EMIE_MMOUSE_LEFT_UP)
                io.MouseDown[2] = false;
            if (event.MouseInput.Event == EMIE_MOUSE_WHEEL)
                io.MouseWheel += event.MouseInput.Wheel;
        }
        
        if (event.EventType == EET_KEY_INPUT_EVENT) {
            // Casting the Irrlicht key directly to ImGuiKey for now to resolve error
            io.AddKeyEvent((ImGuiKey)event.KeyInput.Key, event.KeyInput.PressedDown);
            
            if (event.KeyInput.PressedDown && event.KeyInput.Char != 0) {
                io.AddInputCharacter(event.KeyInput.Char);
            }
        }
        
        wantCaptureMouse = io.WantCaptureMouse;
        wantCaptureKeyboard = io.WantCaptureKeyboard;
    }
};

// Define the bridge function AFTER the class definition
void forwardEventToImGui(const SEvent& event) {
    // Safety check: Only forward events if the ImGui context has been created
    if (ImGui::GetCurrentContext() != nullptr) {
        ImGuiInputHandler::processEvent(event);
    }
}

bool ImGuiInputHandler::wantCaptureMouse = false;
bool ImGuiInputHandler::wantCaptureKeyboard = false;

// --- Main Loop ---

int main() {
    // ... (Keep existing implementation of main)
    Core engine;
    ISceneCollisionManager* coll = engine.smgr->getSceneCollisionManager();

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 130");

    // Scene Setup
    IMeshSceneNode* testMesh = engine.smgr->addCubeSceneNode(10.0f);
    if (testMesh) {
        testMesh->setPosition(vector3df(0, 0, 0));
        testMesh->setMaterialFlag(EMF_LIGHTING, false); 
        // testMesh->getMesh()->setHardwareMappingHint(EHM_DYNAMIC);
    }

    engine.smgr->addLightSceneNode(0, vector3df(0, 20, -20), SColorf(1.0f, 1.0f, 1.0f), 20.0f);

    // Cameras
    ICameraSceneNode* camTop = engine.smgr->addCameraSceneNode(0, vector3df(0, 50, 0), vector3df(0, 0, 0));
    camTop->setUpVector(vector3df(0, 0, 1)); 
    
    ICameraSceneNode* camModel = engine.smgr->addCameraSceneNode(0, vector3df(10, 10, -10), vector3df(0, 0, 0));
    ICameraSceneNode* camFront = engine.smgr->addCameraSceneNode(0, vector3df(0, 0, -50), vector3df(0, 0, 0));
    ICameraSceneNode* camRight = engine.smgr->addCameraSceneNode(0, vector3df(50, 0, 0), vector3df(0, 0, 0));

    matrix4 ortho;
    ortho.buildProjectionMatrixOrthoLH(30, 22, 0, 100); 

    camTop->setProjectionMatrix(ortho, true);
    camFront->setProjectionMatrix(ortho, true);
    camRight->setProjectionMatrix(ortho, true);

    f32 cameraRadius = 20.0f;
    f32 theta = 0.0f;
    f32 phi = 45.0f;
    
    position2di lastMousePos = engine.receiver.MouseState.Position;

    ISceneNode* currentMarker = 0;
    VertexSelection selection;
    bool isDragging = false;
    int activeViewportType = -1;
    
    bool showTestWindow = true;
    int clickCount = 0;
    float cubeScale = 1.0f;

    while(engine.device->run()) {
        
        if (engine.receiver.IsKeyDown(KEY_ESCAPE)) {
            engine.device->closeDevice(); 
        }

        if (engine.device->isWindowActive()) {
            dimension2d<u32> screenSize = engine.driver->getScreenSize();
            s32 w = screenSize.Width;
            s32 h = screenSize.Height;
            s32 midW = w / 2;
            s32 midH = h / 2;

            rect<s32> topLeftRect(0, 0, midW, midH);
            rect<s32> topRightRect(midW, 0, w, midH);
            rect<s32> bottomLeftRect(0, midH, midW, h);
            rect<s32> bottomRightRect(midW, midH, w, h);

            ICameraSceneNode* activeCam = 0;
            rect<s32> activeRect;
            bool inOrthoViewport = false;

            if (isDragging) {
                if (activeViewportType == 0) { 
                    activeCam = camTop; activeRect = topLeftRect; inOrthoViewport = true; 
                } else if (activeViewportType == 1) { 
                    activeCam = camFront; activeRect = bottomLeftRect; inOrthoViewport = true; 
                } else if (activeViewportType == 2) { 
                    activeCam = camRight; activeRect = bottomRightRect; inOrthoViewport = true; 
                }
            } else {
                position2di mPos = engine.receiver.MouseState.Position;
                if (topLeftRect.isPointInside(mPos)) {
                    activeCam = camTop; activeRect = topLeftRect; inOrthoViewport = true; activeViewportType = 0;
                }
                else if (bottomLeftRect.isPointInside(mPos)) {
                    activeCam = camFront; activeRect = bottomLeftRect; inOrthoViewport = true; activeViewportType = 1;
                }
                else if (bottomRightRect.isPointInside(mPos)) {
                    activeCam = camRight; activeRect = bottomRightRect; inOrthoViewport = true; activeViewportType = 2;
                }
                else if (topRightRect.isPointInside(mPos)) {
                    activeViewportType = 3;
                }
                else {
                    activeViewportType = -1; 
                }
            }

            if (!ImGuiInputHandler::wantCaptureMouse && engine.receiver.MouseState.LeftButtonDown) {
                
                if (!isDragging && activeViewportType == 3) {
                    s32 dx = engine.receiver.MouseState.Position.X - lastMousePos.X;
                    s32 dy = engine.receiver.MouseState.Position.Y - lastMousePos.Y;
                    theta -= dx * 0.2f; 
                    phi += dy * 0.2f;  
                    if (phi > 89.0f) phi = 89.0f;
                    if (phi < -89.0f) phi = -89.0f;
                    f32 r = cameraRadius;
                    f32 radTheta = theta * DEGTORAD;
                    f32 radPhi = phi * DEGTORAD;
                    f32 x = r * cos(radPhi) * sin(radTheta);
                    f32 y = r * sin(radPhi);
                    f32 z = r * cos(radPhi) * cos(radTheta) * -1; 
                    camModel->setPosition(vector3df(x, y, z));
                    camModel->setTarget(vector3df(0,0,0)); 
                }

                if (inOrthoViewport && activeCam && testMesh) {
                    
                    if (!isDragging && !selection.isSelected) {
                        vector3df hitPos;
                        u32 bufIdx;
                        std::vector<u32> indices;
                        f32 selectThreshold = 45.0f; 

                        bool found = getClosestVertex(engine.smgr, engine.driver, 
                                                    testMesh, activeCam, 
                                                    activeRect, engine.receiver.MouseState.Position,
                                                    selectThreshold, hitPos, bufIdx, indices);

                        if (found) {
                            selection.isSelected = true;
                            selection.bufferIndex = bufIdx;
                            selection.vertexIndices = indices;
                            selection.worldPos = hitPos;
                            isDragging = true;

                            if (currentMarker) currentMarker->remove();
                            currentMarker = engine.smgr->addSphereSceneNode(0.5f);
                            currentMarker->setPosition(hitPos);
                            currentMarker->setMaterialFlag(EMF_LIGHTING, false);
                            currentMarker->setMaterialFlag(EMF_ZBUFFER, false);
                        }
                    }
                    else if (isDragging && selection.isSelected) {
                        vector3df newPos = getDragPosition(coll, activeCam, 
                                                          engine.receiver.MouseState.Position, 
                                                          selection.worldPos, activeViewportType,
                                                          activeRect);

                        updateVertexPositions(testMesh, selection.bufferIndex, selection.vertexIndices, newPos);
                        
                        selection.worldPos = newPos;
                        if (currentMarker) currentMarker->setPosition(newPos);
                    }
                }
            }
            else if (!engine.receiver.MouseState.LeftButtonDown) {
                isDragging = false;
                selection.isSelected = false;
            }
            
            lastMousePos = engine.receiver.MouseState.Position;

            // Send screen dimensions
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = ImVec2((float)screenSize.Width, (float)screenSize.Height);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui::NewFrame();

            // 2. Render the Toolbar and capture its height
            float toolbarHeight = 0;
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2((float)screenSize.Width, 0)); // Height 0 allows auto-fit

            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | 
                                            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | 
                                            ImGuiWindowFlags_MenuBar;

            if (ImGui::Begin("MainToolbar", nullptr, window_flags)) {
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("File")) {
                        if (ImGui::MenuItem("Exit")) engine.device->closeDevice();
                        ImGui::EndMenu();
                    }
                    // ... other menus
                    ImGui::EndMenuBar();
                }
                toolbarHeight = ImGui::GetWindowHeight(); // This scales with your UI/font
            }
            ImGui::End();

            ImGui::Render();

            engine.driver->beginScene(true, true, SColor(255, 40, 40, 40));

            // Viewport Rendering
            engine.driver->setViewPort(rect<s32>(0, 0, midW, midH));
            engine.smgr->setActiveCamera(camTop);    
            if(testMesh) testMesh->setMaterialFlag(EMF_WIREFRAME, true);
            engine.smgr->drawAll();
            
            engine.driver->setViewPort(rect<s32>(midW, 0, w, midH));
            engine.smgr->setActiveCamera(camModel);
            if(testMesh) testMesh->setMaterialFlag(EMF_WIREFRAME, false);
            engine.smgr->drawAll();

            engine.driver->setViewPort(rect<s32>(0, midH, midW, h));
            engine.smgr->setActiveCamera(camFront);
            if(testMesh) testMesh->setMaterialFlag(EMF_WIREFRAME, true);
            engine.smgr->drawAll();

            engine.driver->setViewPort(rect<s32>(midW, midH, w, h));
            engine.smgr->setActiveCamera(camRight);
            if(testMesh) testMesh->setMaterialFlag(EMF_WIREFRAME, true);
            engine.smgr->drawAll();

            engine.driver->setViewPort(rect<s32>(0, 0, w, h));
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            
            SColor borderColor(255, 100, 100, 100);
            engine.driver->draw2DLine(position2d<s32>(midW, 0), position2d<s32>(midW, h), borderColor);
            engine.driver->draw2DLine(position2d<s32>(0, midH), position2d<s32>(w, midH), borderColor);

            engine.driver->endScene();
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();

    return 0;
}