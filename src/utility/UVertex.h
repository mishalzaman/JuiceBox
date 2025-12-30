#include <irrlicht.h>
#include <cmath>
#include <vector>

#include "Camera.h"

using namespace irr;
using namespace core;
using namespace video;

struct VertexSelection {
    bool isSelected = false;
    u32 bufferIndex = 0;
    std::vector<u32> vertexIndices;
    vector3df worldPos;
};

namespace UVertex {
    inline constexpr f32 DEFAULT_SELECT_THRESHOLD = 45.0f;
    inline constexpr f32 POSITION_EPSILON = 0.001f;

    inline bool FindClosestVertex(
        IMeshSceneNode* node, 
        ICameraSceneNode* camera, 
        rect<s32> viewport, 
        position2di mousePos,
        f32 pixelThreshold, 
        vector3df& outPos,
        u32& outBufferIndex, 
        std::vector<u32>& outIndices
    ) {
        bool found = false;
        f32 minDistSq = pixelThreshold * pixelThreshold;
        f32 closestDepthSq = FLT_MAX; 
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

        u32 closestBufferIndex = 0;
        vector3df closestWorldPos;
        vector3df closestLocalPos;

        // First pass: find the closest vertex to mouse position
        for (u32 b = 0; b < mesh->getMeshBufferCount(); ++b) {
            IMeshBuffer* mb = mesh->getMeshBuffer(b);
            
            // Only handle standard vertex type
            if (mb->getVertexType() != EVT_STANDARD) {
                continue;
            }
            
            S3DVertex* vertices = (S3DVertex*)mb->getVertices();

            for (u32 v = 0; v < mb->getVertexCount(); ++v) {
                vector3df localPos = vertices[v].Pos;
                vector3df worldPos = localPos;
                world.transformVect(worldPos);

                f32 transformedPos[4] = { worldPos.X, worldPos.Y, worldPos.Z, 1.0f };
                viewProj.multiplyWith1x4Matrix(transformedPos);

                if (transformedPos[3] == 0) continue;

                f32 zDiv = 1.0f / transformedPos[3];
                f32 ndcX = transformedPos[0] * zDiv;
                f32 ndcY = transformedPos[1] * zDiv;

                f32 screenX = (ndcX + 1.0f) * 0.5f * vpW + vpX;
                f32 screenY = (1.0f - ndcY) * 0.5f * vpH + vpY;

                f32 dx = screenX - mousePos.X;
                f32 dy = screenY - mousePos.Y;
                f32 distSq = dx * dx + dy * dy;

                if (distSq < minDistSq) {
                    f32 depthSq = worldPos.getDistanceFromSQ(camPos);
                    if (depthSq < closestDepthSq) {
                        closestDepthSq = depthSq;
                        closestWorldPos = worldPos;
                        closestLocalPos = localPos;
                        closestBufferIndex = b;
                        found = true;
                    }
                }
            }
        }

        // Second pass: find all vertices at the same position (welded vertices)
        if (found) {
            outPos = closestWorldPos;
            outBufferIndex = closestBufferIndex;
            
            IMeshBuffer* mb = mesh->getMeshBuffer(closestBufferIndex);
            S3DVertex* vertices = (S3DVertex*)mb->getVertices();

            for (u32 v = 0; v < mb->getVertexCount(); ++v) {
                if (vertices[v].Pos.equals(closestLocalPos, POSITION_EPSILON)) {
                    outIndices.push_back(v);
                }
            }
        }

        return found;
    };

    inline VertexSelection Select(
        IMeshSceneNode* mesh,
        ICameraSceneNode* camera,
        rect<s32> viewportSegment,
        position2di mousePos,
        EditorMode mode
    ) {
        VertexSelection selection;
        
        if (!mesh || !camera) {
            return selection;
        }
        
        vector3df hitPos;
        u32 bufIdx;
        std::vector<u32> indices;

        bool found = FindClosestVertex(
            mesh,
            camera, 
            viewportSegment,
            mousePos,
            DEFAULT_SELECT_THRESHOLD,
            hitPos,
            bufIdx,
            indices
        );

        if (found) {
            selection.isSelected = true;
            selection.bufferIndex = bufIdx;
            selection.vertexIndices = indices;
            selection.worldPos = hitPos;
        }

        return selection;
    };

    inline vector3df Move(
        ISceneCollisionManager* coll, 
        ICameraSceneNode* camera, 
        position2di mousePos, 
        vector3df originalPos, 
        int ViewportType,  
        rect<s32> viewport
    ) {
        int index = (ViewportType >= 0 && ViewportType <= 5) ? ViewportType : 0;
        plane3df dragPlane(originalPos, ViewportCameraNormals[index]);

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
    };
}