#pragma once

#include <irrlicht.h>

using namespace irr;
using namespace core;
using namespace video;

namespace Mesh {
    inline void Update(
        IMeshSceneNode* node,
        u32 bufferIndex,
        const std::vector<u32>& indices,
        vector3df worldPos
    ) {
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
}
