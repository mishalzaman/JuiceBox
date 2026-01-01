#pragma once

#include <irrlicht.h>
#include <cmath>
#include <vector>
#include <iostream>
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
        void UpdateMesh(vector3df vertexCurrent, vector3df vertexNew);

        // Vertices
        void AddSelectedVertex(vector3df position);
        std::vector<ISceneNode*> GetSelectedVertices();
        void ClearSelectedVertices();

        void ClearAll();
    private:
        Application& _application;
        IMeshSceneNode* _mesh;
        ISceneCollisionManager* _collisionManager;

        // Vertices
        std::vector<ISceneNode*> _selectedVertices;
};