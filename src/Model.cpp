#include "Model.h"

Model::Model(Application &application)
:_application(application),
_highlightedVertex(nullptr),
_selectedVertices({})
{
}

Model::~Model()
{
}

void Model::GenerateDefault()
{
    _collisionManager = _application.smgr->getSceneCollisionManager();
    SMesh* cubeMesh = Mesh::CreateCube(8.0f);

    _mesh = _application.smgr->addMeshSceneNode(cubeMesh);
    
    cubeMesh->drop();

    if (_mesh) {
        _mesh->setPosition(vector3df(0, 0, 0));
        _mesh->setMaterialFlag(EMF_LIGHTING, false);
    }

    _application.smgr->addLightSceneNode(0, vector3df(0, 20, -20), SColorf(1.0f, 1.0f, 1.0f), 20.0f);
}

void Model::SetHighlightedVertex(vector3df position)
{
    ClearHighlighted(); // Remove existing highlight first to prevent memory leak
    
    _highlightedVertex = _application.smgr->addCubeSceneNode(0.5f);
    _highlightedVertex->setPosition(position);
    _highlightedVertex->setMaterialFlag(EMF_LIGHTING, false);
    _highlightedVertex->setMaterialFlag(EMF_ZBUFFER, false);
}

void Model::AddSelectedVertex()
{
    ISceneNode* selected = _application.smgr->addCubeSceneNode(0.5f);
    selected->setPosition(_highlightedVertex->getPosition());
    selected->setMaterialFlag(EMF_LIGHTING, true);
    selected->setMaterialFlag(EMF_ZBUFFER, false);
    selected->setMaterialFlag(EMF_ZWRITE_ENABLE, false);

    selected->getMaterial(0).EmissiveColor.set(255, 0, 255, 0);

    _selectedVertices.push_back(selected);

    ClearHighlighted();
}

std::vector<ISceneNode *> Model::GetSelectedVertices()
{
    return _selectedVertices;
}

void Model::ClearSelectedVertices()
{
    for (ISceneNode* node : _selectedVertices) {
        if (node) {
            node->remove();
        }
    }
    _selectedVertices.clear();
}

void Model::ClearAll()
{
    ClearSelectedVertices();
}

void Model::ClearHighlighted()
{
    if (_highlightedVertex) {
        _highlightedVertex->remove();
        _highlightedVertex = nullptr;
    }
}
