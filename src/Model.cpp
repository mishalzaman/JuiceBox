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
        
        // Load and apply the crate texture
        ITexture* crateTexture = _application.driver->getTexture("assets/crate.png");
        if (crateTexture) {
            _mesh->setMaterialTexture(0, crateTexture);
        }
    }

    _application.smgr->addLightSceneNode(0, vector3df(0, 10, -10), SColorf(1.0f, 1.0f, 1.0f), 40.0f);
}

void Model::UpdateMesh(vector3df vertexCurrent, vector3df vertexNew)
{
    if (!_mesh) 
        return;

    IMesh* mesh = _mesh->getMesh();
    u32 bufferCount = mesh->getMeshBufferCount();

    // Iterate through all mesh buffers (a mesh can have multiple)
    for (u32 i = 0; i < bufferCount; ++i) 
    {
        IMeshBuffer* mb = mesh->getMeshBuffer(i);
        
        // Ensure we are working with standard vertices
        if (mb->getVertexType() == EVT_STANDARD) 
        {
            S3DVertex* vertices = (S3DVertex*)mb->getVertices();
            u32 vertexCount = mb->getVertexCount();

            for (u32 j = 0; j < vertexCount; ++j) 
            {
                // Use an epsilon check for floating point comparison
                if (vertices[j].Pos.equals(vertexCurrent, 0.001f)) 
                {
                    vertices[j].Pos = vertexNew;
                }
            }
        }
        // Handle other vertex types if necessary (e.g., 2TCoords or Tangents)
        else if (mb->getVertexType() == EVT_2TCOORDS)
        {
            S3DVertex2TCoords* vertices = (S3DVertex2TCoords*)mb->getVertices();
            for (u32 j = 0; j < mb->getVertexCount(); ++j)
            {
                if (vertices[j].Pos.equals(vertexCurrent, 0.001f))
                    vertices[j].Pos = vertexNew;
            }
        }
    }

    // CRITICAL: Inform the hardware that the vertex data has changed
    _mesh->getMesh()->setDirty(EBT_VERTEX);
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
