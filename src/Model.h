#pragma once

#include <irrlicht.h>
#include <cmath>
#include <vector>
#include "Application.h"
#include "helpers/Mesh.h"

using namespace irr;
using namespace core;
using namespace scene;

class Model {
    public:
        Model(Application& application);
        ~Model();
        IMeshSceneNode* GetMesh() { return _mesh; }
        void GenerateDefault();

        // Vertices
        void SetHighlightedVertex(vector3df position);
        bool HasHighlightedVertex() { return !!_highlightedVertex; };
        void AddSelectedVertex();
        std::vector<ISceneNode*> GetSelectedVertices();
        void ClearSelectedVertices();

        void ClearAll();
        void ClearHighlighted();
    private:
        Application& _application;
        IMeshSceneNode* _mesh;
        ISceneCollisionManager* _collisionManager;

        // Vertices
        ISceneNode* _highlightedVertex;
        std::vector<ISceneNode*> _selectedVertices;
};