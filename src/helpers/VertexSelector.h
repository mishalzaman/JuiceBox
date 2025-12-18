#pragma once

#include <irrlicht.h>

using namespace irr;
using namespace core;
using namespace video;

namespace VertexSelector {
    inline bool Get(ISceneManager* smgr, IVideoDriver* driver, 
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
}
