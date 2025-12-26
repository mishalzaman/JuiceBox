#pragma once

#include <irrlicht.h>
#include <cmath>
#include <vector>
#include "Application.h"

using namespace irr;
using namespace core;
using namespace scene;

class Model {
    public:
        Model(Application& application);
        ~Model();
        void GenerateDefault();
        void ClearVertices();
        void ClearAll();
    private:
        Application& _application;

        // Vertices
        ISceneNode* _highlightedVertex;
        std::vector<ISceneNode*> _selectedVertex;
};