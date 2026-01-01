#include <irrlicht.h>
#include <cmath>
#include <vector>

#include "Camera.h"

using namespace irr;
using namespace core;
using namespace video;

namespace UVertex {
    inline constexpr f32 DEFAULT_SELECT_THRESHOLD = 45.0f;
    inline constexpr f32 POSITION_EPSILON = 0.001f;
    inline constexpr f32 EDGE_SELECT_THRESHOLD = 10.0f;

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

        for (u32 b = 0; b < mesh->getMeshBufferCount(); ++b) {
            IMeshBuffer* mb = mesh->getMeshBuffer(b);
            
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

    inline f32 PointToLineSegmentDistanceSq(
        vector2df point,
        vector2df lineStart,
        vector2df lineEnd
    ) {
        vector2df line = lineEnd - lineStart;
        f32 lineLengthSq = line.getLengthSQ();
        
        if (lineLengthSq < 0.0001f) {
            return point.getDistanceFromSQ(lineStart);
        }

        vector2df pointVec = point - lineStart;
        f32 t = pointVec.dotProduct(line) / lineLengthSq;
        t = core::clamp(t, 0.0f, 1.0f);
        
        vector2df projection = lineStart + line * t;
        return point.getDistanceFromSQ(projection);
    };

    inline EdgeSelection FindClosestEdge(
        IMeshSceneNode* node,
        ICameraSceneNode* camera,
        rect<s32> viewport,
        position2di mousePos,
        f32 pixelThreshold
    ) {
        EdgeSelection result;
        f32 minDistSq = pixelThreshold * pixelThreshold;
        f32 closestDepthSq = FLT_MAX;

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
        vector2df mousePosF((f32)mousePos.X, (f32)mousePos.Y);

        auto worldToScreen = [&](const vector3df& worldPos, vector2df& screenPos) -> bool {
            f32 transformedPos[4] = { worldPos.X, worldPos.Y, worldPos.Z, 1.0f };
            viewProj.multiplyWith1x4Matrix(transformedPos);

            if (transformedPos[3] == 0) return false;

            f32 zDiv = 1.0f / transformedPos[3];
            f32 ndcX = transformedPos[0] * zDiv;
            f32 ndcY = transformedPos[1] * zDiv;

            screenPos.X = (ndcX + 1.0f) * 0.5f * vpW + vpX;
            screenPos.Y = (1.0f - ndcY) * 0.5f * vpH + vpY;
            return true;
        };

        for (u32 b = 0; b < mesh->getMeshBufferCount(); ++b) {
            IMeshBuffer* mb = mesh->getMeshBuffer(b);
            
            if (mb->getVertexType() != EVT_STANDARD) {
                continue;
            }

            S3DVertex* vertices = (S3DVertex*)mb->getVertices();
            u16* indices = mb->getIndices();
            u32 indexCount = mb->getIndexCount();

            for (u32 i = 0; i < indexCount; i += 3) {
                for (u32 e = 0; e < 3; ++e) {
                    u32 idx1 = indices[i + e];
                    u32 idx2 = indices[i + (e + 1) % 3];

                    vector3df localPos1 = vertices[idx1].Pos;
                    vector3df localPos2 = vertices[idx2].Pos;

                    vector3df worldPos1 = localPos1;
                    vector3df worldPos2 = localPos2;
                    world.transformVect(worldPos1);
                    world.transformVect(worldPos2);

                    vector2df screenPos1, screenPos2;
                    if (!worldToScreen(worldPos1, screenPos1)) continue;
                    if (!worldToScreen(worldPos2, screenPos2)) continue;

                    f32 distSq = PointToLineSegmentDistanceSq(mousePosF, screenPos1, screenPos2);

                    if (distSq < minDistSq) {
                        vector3df midpoint = (worldPos1 + worldPos2) * 0.5f;
                        f32 depthSq = midpoint.getDistanceFromSQ(camPos);

                        if (depthSq < closestDepthSq) {
                            closestDepthSq = depthSq;
                            result.isSelected = true;
                            result.bufferIndex = b;
                            result.vertexIndex1 = idx1;
                            result.vertexIndex2 = idx2;
                            result.worldPos1 = worldPos1;
                            result.worldPos2 = worldPos2;
                        }
                    }
                }
            }
        }

        return result;
    };

inline FaceSelection FindClosestFace(
    IMeshSceneNode* node,
    ICameraSceneNode* camera,
    rect<s32> viewport,
    position2di mousePos
) {
    FaceSelection result;
    f32 minDistSq = FLT_MAX;
    f32 closestDepthSq = FLT_MAX;

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
    vector2df mousePosF((f32)mousePos.X, (f32)mousePos.Y);

    auto worldToScreen = [&](const vector3df& worldPos, vector2df& screenPos) -> bool {
        f32 transformedPos[4] = { worldPos.X, worldPos.Y, worldPos.Z, 1.0f };
        viewProj.multiplyWith1x4Matrix(transformedPos);

        if (transformedPos[3] == 0) return false;

        f32 zDiv = 1.0f / transformedPos[3];
        f32 ndcX = transformedPos[0] * zDiv;
        f32 ndcY = transformedPos[1] * zDiv;

        screenPos.X = (ndcX + 1.0f) * 0.5f * vpW + vpX;
        screenPos.Y = (1.0f - ndcY) * 0.5f * vpH + vpY;
        return true;
    };

    // Check if point is inside triangle (2D)
    auto pointInTriangle = [](const vector2df& p, const vector2df& a, const vector2df& b, const vector2df& c) -> bool {
        auto sign = [](const vector2df& p1, const vector2df& p2, const vector2df& p3) -> f32 {
            return (p1.X - p3.X) * (p2.Y - p3.Y) - (p2.X - p3.X) * (p1.Y - p3.Y);
        };

        f32 d1 = sign(p, a, b);
        f32 d2 = sign(p, b, c);
        f32 d3 = sign(p, c, a);

        bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

        return !(hasNeg && hasPos);
    };

    for (u32 b = 0; b < mesh->getMeshBufferCount(); ++b) {
        IMeshBuffer* mb = mesh->getMeshBuffer(b);
        
        if (mb->getVertexType() != EVT_STANDARD) {
            continue;
        }

        S3DVertex* vertices = (S3DVertex*)mb->getVertices();
        u16* indices = mb->getIndices();
        u32 indexCount = mb->getIndexCount();

        for (u32 i = 0; i < indexCount; i += 3) {
            u32 idx1 = indices[i];
            u32 idx2 = indices[i + 1];
            u32 idx3 = indices[i + 2];

            vector3df localPos1 = vertices[idx1].Pos;
            vector3df localPos2 = vertices[idx2].Pos;
            vector3df localPos3 = vertices[idx3].Pos;

            vector3df worldPos1 = localPos1;
            vector3df worldPos2 = localPos2;
            vector3df worldPos3 = localPos3;
            world.transformVect(worldPos1);
            world.transformVect(worldPos2);
            world.transformVect(worldPos3);

            vector2df screenPos1, screenPos2, screenPos3;
            if (!worldToScreen(worldPos1, screenPos1)) continue;
            if (!worldToScreen(worldPos2, screenPos2)) continue;
            if (!worldToScreen(worldPos3, screenPos3)) continue;

            // Check if mouse is inside the triangle
            if (pointInTriangle(mousePosF, screenPos1, screenPos2, screenPos3)) {
                // Calculate centroid for depth comparison
                vector3df centroid = (worldPos1 + worldPos2 + worldPos3) / 3.0f;
                f32 depthSq = centroid.getDistanceFromSQ(camPos);

                if (depthSq < closestDepthSq) {
                    closestDepthSq = depthSq;
                    result.isSelected = true;
                    result.bufferIndex = b;
                    result.vertexIndex1 = idx1;
                    result.vertexIndex2 = idx2;
                    result.vertexIndex3 = idx3;
                    result.worldPos1 = worldPos1;
                    result.worldPos2 = worldPos2;
                    result.worldPos3 = worldPos3;
                }
            }
        }
    }

    return result;
}

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

    inline EdgeSelection SelectEdge(
        IMeshSceneNode* mesh,
        ICameraSceneNode* camera,
        rect<s32> viewportSegment,
        position2di mousePos
    ) {
        if (!mesh || !camera) {
            return EdgeSelection();
        }

        return FindClosestEdge(
            mesh,
            camera,
            viewportSegment,
            mousePos,
            EDGE_SELECT_THRESHOLD
        );
    };

    inline FaceSelection SelectFace(
        IMeshSceneNode* mesh,
        ICameraSceneNode* camera,
        rect<s32> viewportSegment,
        position2di mousePos
    ) {
        if (!mesh || !camera) {
            return FaceSelection();
        }

        return FindClosestFace(
            mesh,
            camera,
            viewportSegment,
            mousePos
        );
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