#pragma once

#include <irrlicht.h>
#include <cmath>
#include <vector>
#include "Application.h"

using namespace irr;
using namespace core;
using namespace video;

struct VertexSelection {
    bool isSelected = false;
    u32 bufferIndex = 0;
    std::vector<u32> vertexIndices;
    vector3df worldPos;
};

namespace Mode {
    class Vertex {
        public:
            Vertex(Application& application);
            ~Vertex();

            VertexSelection Select(
                IMeshSceneNode* mesh,
                ICameraSceneNode* camera,
                rect<s32> viewportSegment
            );
        private:      
            static constexpr f32 DEFAULT_SELECT_THRESHOLD = 45.0f;
            static constexpr f32 POSITION_EPSILON = 0.001f;

            bool _findClosestVertex(
                IMeshSceneNode* node, 
                ICameraSceneNode* camera, 
                rect<s32> viewport, 
                position2di mousePos,
                f32 pixelThreshold, 
                vector3df& outPos,
                u32& outBufferIndex, 
                std::vector<u32>& outIndices
            );

            Application& _application;
    };
};