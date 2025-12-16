#include <irrlicht.h>
#include <iostream>
#include <cmath>
#include <vector> // Required for storing multiple vertex indices

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

// Structure to track selected vertex (and all its connected duplicates)
struct VertexSelection {
    bool isSelected = false;
    u32 bufferIndex = 0;
    std::vector<u32> vertexIndices; // Stores ALL indices sharing the same position
    vector3df worldPos;
};

// Returns true if a vertex was found within the threshold
bool getClosestVertex(ISceneManager* smgr, IVideoDriver* driver, 
                      IMeshSceneNode* node, ICameraSceneNode* camera, 
                      rect<s32> viewport, position2di mousePos,
                      f32 pixelThreshold, vector3df& outPos,
                      u32& outBufferIndex, std::vector<u32>& outIndices)
{
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

    // 1. First Pass: Find the closest vertex to the mouse visually
    u32 closestVIndex = 0;
    u32 closestBIndex = 0;
    vector3df closestWorldPos;

    for (u32 b = 0; b < mesh->getMeshBufferCount(); ++b) {
        IMeshBuffer* mb = mesh->getMeshBuffer(b);
        S3DVertex* vertices = (S3DVertex*)mb->getVertices();

        for (u32 v = 0; v < mb->getVertexCount(); ++v) {
            vector3df vPos = vertices[v].Pos;
            world.transformVect(vPos); // To World Space

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

    // 2. Second Pass: Find ALL vertices that share this position (Weld Logic)
    if (found) {
        outPos = closestWorldPos;
        outBufferIndex = closestBIndex;
        
        IMeshBuffer* mb = mesh->getMeshBuffer(closestBIndex);
        S3DVertex* vertices = (S3DVertex*)mb->getVertices();
        
        // Transform back to local space to compare with raw vertex data
        matrix4 invWorld;
        node->getAbsoluteTransformation().getInverse(invWorld);
        vector3df targetLocalPos = closestWorldPos;
        invWorld.transformVect(targetLocalPos);

        for (u32 v = 0; v < mb->getVertexCount(); ++v) {
            // Check if positions match (using a small epsilon for float errors)
            if (vertices[v].Pos.equals(targetLocalPos, 0.001f)) {
                outIndices.push_back(v);
            }
        }
    }

    return found;
}

// DRAGGING FIX: Use Plane Intersection (Ray-Cast) instead of Unprojection
// This prevents the mesh from jumping around or sliding in depth
vector3df getDragPosition(ISceneCollisionManager* coll, ICameraSceneNode* camera, 
                          position2di mousePos, vector3df originalPos, int viewportType)
{
    // 1. Create a plane passing through the selected vertex
    //    The Normal determines which axis is LOCKED.
    plane3df dragPlane;

    switch(viewportType) {
        case 0: // Top View (Looking down Y) -> Plane is XZ (Normal Y)
            dragPlane = plane3df(originalPos, vector3df(0, 1, 0));
            break;
        case 1: // Front View (Looking forward Z) -> Plane is XY (Normal Z)
            dragPlane = plane3df(originalPos, vector3df(0, 0, -1));
            break;
        case 2: // Right View (Looking left X) -> Plane is YZ (Normal X)
            dragPlane = plane3df(originalPos, vector3df(1, 0, 0));
            break;
        default:
            return originalPos;
    }

    // 2. Get a ray from the camera through the mouse cursor
    line3d<f32> ray = coll->getRayFromScreenCoordinates(mousePos, camera);

    // 3. Intersect ray with plane
    vector3df intersection;
    if (dragPlane.getIntersectionWithLine(ray.start, ray.getVector(), intersection)) {
        return intersection;
    }

    return originalPos;
}

// Update ALL selected vertices to the new position
void updateVertexPositions(IMeshSceneNode* node, u32 bufferIndex, const std::vector<u32>& indices, vector3df worldPos)
{
    IMesh* mesh = node->getMesh();
    if (bufferIndex >= mesh->getMeshBufferCount()) return;
    
    IMeshBuffer* mb = mesh->getMeshBuffer(bufferIndex);
    S3DVertex* vertices = (S3DVertex*)mb->getVertices();
    
    // Convert world position back to local space ONCE
    matrix4 worldTransform = node->getAbsoluteTransformation();
    matrix4 invWorld;
    worldTransform.getInverse(invWorld);
    
    vector3df localPos = worldPos;
    invWorld.transformVect(localPos);
    
    // Update all linked vertices
    for (u32 idx : indices) {
        if (idx < mb->getVertexCount()) {
            vertices[idx].Pos = localPos;
        }
    }
    
    mb->setDirty(EBT_VERTEX);
    mesh->setDirty();
}

int main() {
    Core engine;
    ISceneCollisionManager* coll = engine.smgr->getSceneCollisionManager();

    // --- Scene Setup ---
    // You can use a Cube now! The "Weld Logic" in getClosestVertex handles the split corners.
    IMeshSceneNode* testMesh = engine.smgr->addCubeSceneNode(10.0f);
    if (testMesh) {
        testMesh->setPosition(vector3df(0, 0, 0));
        testMesh->setMaterialFlag(EMF_LIGHTING, false); 
        testMesh->getMesh()->setHardwareMappingHint(EHM_DYNAMIC);
    }

    engine.smgr->addLightSceneNode(0, vector3df(0, 20, -20), SColorf(1.0f, 1.0f, 1.0f), 20.0f);

    // --- Cameras ---
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

    // --- Orbit Control Variables ---
    f32 cameraRadius = 20.0f;
    f32 theta = 0.0f;
    f32 phi = 45.0f;
    
    position2di lastMousePos = engine.receiver.MouseState.Position;

    ISceneNode* currentMarker = 0;
    VertexSelection selection;
    bool isDragging = false;
    
    // State Persistence (Fixes logic drift)
    int activeViewportType = -1; // -1=none, 0=top, 1=front, 2=right, 3=model

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

            /*==========================================
             STATE MANAGEMENT
            ===========================================*/
            // If dragging, LOCK logic to the starting viewport
            if (isDragging) {
                if (activeViewportType == 0) { 
                    activeCam = camTop; activeRect = topLeftRect; inOrthoViewport = true; 
                } else if (activeViewportType == 1) { 
                    activeCam = camFront; activeRect = bottomLeftRect; inOrthoViewport = true; 
                } else if (activeViewportType == 2) { 
                    activeCam = camRight; activeRect = bottomRightRect; inOrthoViewport = true; 
                }
            } else {
                // Hit testing
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
                    activeViewportType = 3; // Model view
                }
                else {
                    activeViewportType = -1; 
                }
            }

            /*==========================================
             MOUSE HANDLING
            ===========================================*/
            if (engine.receiver.MouseState.LeftButtonDown) {
                
                // --- ORBIT CONTROL (Model View) ---
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

                // --- VERTEX OPERATIONS (Ortho Viewports) ---
                if (inOrthoViewport && activeCam && testMesh) {
                    
                    // 1. SELECT
                    if (!isDragging && !selection.isSelected) {
                        vector3df hitPos;
                        u32 bufIdx;
                        std::vector<u32> indices; // Hold all coincident vertices
                        f32 selectThreshold = 45.0f; 

                        bool found = getClosestVertex(engine.smgr, engine.driver, 
                                                    testMesh, activeCam, 
                                                    activeRect, engine.receiver.MouseState.Position,
                                                    selectThreshold, hitPos, bufIdx, indices);

                        if (found) {
                            std::cout << "Selected " << indices.size() << " vertices at: " << hitPos.X << ", " << hitPos.Y << ", " << hitPos.Z << std::endl;
                            
                            selection.isSelected = true;
                            selection.bufferIndex = bufIdx;
                            selection.vertexIndices = indices; // Store the list!
                            selection.worldPos = hitPos;
                            isDragging = true;

                            if (currentMarker) currentMarker->remove();
                            currentMarker = engine.smgr->addSphereSceneNode(0.5f);
                            currentMarker->setPosition(hitPos);
                            currentMarker->setMaterialFlag(EMF_LIGHTING, false);
                            currentMarker->setMaterialFlag(EMF_ZBUFFER, false);
                        }
                    }
                    // 2. DRAG
                    else if (isDragging && selection.isSelected) {
                        
                        // Temporarily set viewport for correct ray projection
                        rect<s32> oldVP = engine.driver->getViewPort();
                        engine.driver->setViewPort(activeRect);
                        
                        // Use Plane Intersection for stable movement
                        vector3df newPos = getDragPosition(coll, activeCam, 
                                                          engine.receiver.MouseState.Position, 
                                                          selection.worldPos, activeViewportType);
                        
                        engine.driver->setViewPort(oldVP);

                        // Update ALL vertices in the list
                        updateVertexPositions(testMesh, selection.bufferIndex, selection.vertexIndices, newPos);
                        
                        selection.worldPos = newPos;
                        if (currentMarker) currentMarker->setPosition(newPos);
                    }
                }
            }
            else {
                isDragging = false;
                selection.isSelected = false;
            }
            
            lastMousePos = engine.receiver.MouseState.Position;

            engine.driver->beginScene(true, true, SColor(255, 40, 40, 40));

            // --- DRAW VIEWPORTS ---
            
            // TOP LEFT (Top View)
            engine.driver->setViewPort(rect<s32>(0, 0, midW, midH));
            engine.smgr->setActiveCamera(camTop);    
            if(testMesh) {
                testMesh->setMaterialFlag(EMF_WIREFRAME, true);
                testMesh->setMaterialFlag(EMF_LIGHTING, false);
            }
            engine.smgr->drawAll();
            
            // TOP RIGHT (Model View)
            engine.driver->setViewPort(rect<s32>(midW, 0, w, midH));
            engine.smgr->setActiveCamera(camModel);
            if(testMesh) {
                testMesh->setMaterialFlag(EMF_WIREFRAME, false);
                testMesh->setMaterialFlag(EMF_LIGHTING, true);
            }
            engine.smgr->drawAll();

            // BOTTOM LEFT (Front View)
            engine.driver->setViewPort(rect<s32>(0, midH, midW, h));
            engine.smgr->setActiveCamera(camFront);
            if(testMesh) {
                testMesh->setMaterialFlag(EMF_WIREFRAME, true);
                testMesh->setMaterialFlag(EMF_LIGHTING, false);
            }
            engine.smgr->drawAll();

            // BOTTOM RIGHT (Right View)
            engine.driver->setViewPort(rect<s32>(midW, midH, w, h));
            engine.smgr->setActiveCamera(camRight);
            if(testMesh) {
                testMesh->setMaterialFlag(EMF_WIREFRAME, true);
                testMesh->setMaterialFlag(EMF_LIGHTING, false);
            }
            engine.smgr->drawAll();

            // --- DRAW BORDERS ---
            engine.driver->setViewPort(rect<s32>(0, 0, w, h));
            SColor borderColor(255, 100, 100, 100);
            engine.driver->draw2DLine(position2d<s32>(midW, 0), position2d<s32>(midW, h), borderColor);
            engine.driver->draw2DLine(position2d<s32>(0, midH), position2d<s32>(w, midH), borderColor);

            engine.driver->endScene();
        }
    }

    return 0;
}