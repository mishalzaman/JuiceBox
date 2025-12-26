#include "Model.h"

Model::Model(Application &application)
:_application(application),
_highlightedVertex(nullptr),
_selectedVertex({})
{
}

Model::~Model()
{
}

void Model::GenerateDefault()
{
}

void Model::ClearVertices()
{
}

void Model::ClearAll()
{
}
