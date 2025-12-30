#pragma once

#include <irrlicht.h>
#include <vector>

using namespace irr;
using namespace core;
using namespace video;
using namespace scene;

namespace Mesh {
    inline SMesh* CreateCube(f32 size) {
        SMesh* mesh = new SMesh();
        SMeshBuffer* buffer = new SMeshBuffer();

        f32 s = size / 2.0f;
        SColor white(255, 255, 255, 255);

        // 8 Shared Vertices
        buffer->Vertices.push_back(S3DVertex(-s,-s, s, -1,-1, 1, white, 0, 1)); // 0
        buffer->Vertices.push_back(S3DVertex( s,-s, s,  1,-1, 1, white, 1, 1)); // 1
        buffer->Vertices.push_back(S3DVertex( s, s, s,  1, 1, 1, white, 1, 0)); // 2
        buffer->Vertices.push_back(S3DVertex(-s, s, s, -1, 1, 1, white, 0, 0)); // 3
        buffer->Vertices.push_back(S3DVertex( s,-s,-s,  1,-1,-1, white, 0, 1)); // 4
        buffer->Vertices.push_back(S3DVertex(-s,-s,-s, -1,-1,-1, white, 1, 1)); // 5
        buffer->Vertices.push_back(S3DVertex(-s, s,-s, -1, 1,-1, white, 1, 0)); // 6
        buffer->Vertices.push_back(S3DVertex( s, s,-s,  1, 1,-1, white, 0, 0)); // 7

        u16 u_indices[] = {
            0,1,2, 0,2,3, // Front
            1,4,7, 1,7,2, // Right
            4,5,6, 4,6,7, // Back
            5,0,3, 5,3,6, // Left
            3,2,7, 3,7,6, // Top
            0,5,4, 0,4,1  // Bottom
        };

        for (u32 i = 0; i < 36; ++i) buffer->Indices.push_back(u_indices[i]);

        mesh->addMeshBuffer(buffer);
        buffer->drop();
        mesh->recalculateBoundingBox();
        return mesh;
    }

    inline SMesh* CreateSphere(f32 radius, u32 polyCountX = 16, u32 polyCountY = 16) {
        SMesh* mesh = new SMesh();
        SMeshBuffer* buffer = new SMeshBuffer();
        
        SColor white(255, 255, 255, 255);

        // 1. Generate Vertices
        for (u32 y = 0; y <= polyCountY; ++y) {
            // Angle from 0 (top pole) to PI (bottom pole)
            f64 phi = core::PI * (f64)y / (f64)polyCountY;
            f32 sinPhi = sin(phi);
            f32 cosPhi = cos(phi);

            for (u32 x = 0; x <= polyCountX; ++x) {
                // Angle from 0 to 2*PI around the equator
                f64 theta = 2.0 * core::PI * (f64)x / (f64)polyCountX;
                f32 sinTheta = sin(theta);
                f32 cosTheta = cos(theta);

                // Cartesian coordinates
                vector3df pos(
                    radius * sinPhi * cosTheta,
                    radius * cosPhi,
                    radius * sinPhi * sinTheta
                );

                // Normal is just the normalized position for a sphere centered at (0,0,0)
                vector3df normal = pos;
                normal.normalize();

                // UV Mapping
                f32 tu = (f32)x / (f32)polyCountX;
                f32 tv = (f32)y / (f32)polyCountY;

                buffer->Vertices.push_back(S3DVertex(pos, normal, white, vector2df(tu, tv)));
            }
        }

        // 2. Generate Indices (Triangles)
        for (u32 y = 0; y < polyCountY; ++y) {
            for (u32 x = 0; x < polyCountX; ++x) {
                u32 current = y * (polyCountX + 1) + x;
                u32 next = current + polyCountX + 1;

                // Triangle 1
                buffer->Indices.push_back(current);
                buffer->Indices.push_back(next);
                buffer->Indices.push_back(current + 1);

                // Triangle 2
                buffer->Indices.push_back(next);
                buffer->Indices.push_back(next + 1);
                buffer->Indices.push_back(current + 1);
            }
        }

        mesh->addMeshBuffer(buffer);
        buffer->drop();
        mesh->recalculateBoundingBox();
        return mesh;
    }

    inline void Update(
        IMeshSceneNode* node,
        u32 bufferIndex,
        const std::vector<u32>& indices,
        vector3df worldPos
    ) {
        IMesh* mesh = node->getMesh();
        if (!mesh || bufferIndex >= mesh->getMeshBufferCount()) return;
        
        IMeshBuffer* mb = mesh->getMeshBuffer(bufferIndex);
        S3DVertex* vertices = (S3DVertex*)mb->getVertices();
        
        matrix4 invWorld;
        node->getAbsoluteTransformation().getInverse(invWorld);
        vector3df localPos = worldPos;
        invWorld.transformVect(localPos);
        
        for (u32 idx : indices) {
            if (idx < mb->getVertexCount()) {
                vertices[idx].Pos = localPos;
            }
        }
        
        // 1. Mark vertex buffer as changed for the GPU
        mb->setDirty(EBT_VERTEX);

        // 2. Recalculate the buffer's individual bounding box
        mb->recalculateBoundingBox();

        // 3. Recalculate the overall mesh bounding box (Cast required)
        static_cast<SMesh*>(mesh)->recalculateBoundingBox();

        // 4. Update the Scene Node's box
        // We don't call setBoundingBox; we tell the node to refresh from the mesh
        node->setAutomaticCulling(scene::EAC_BOX); 
    }   
}