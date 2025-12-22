#include "Editor.h"
#include <iostream>

const vector3df Editor::CAMERA_LOOKAT = vector3df(0, 0, 0);
const vector3df Editor::CAMERA_TOP_POS = vector3df(0, 50, 0);
const vector3df Editor::CAMERA_TOP_UP = vector3df(0, 0, 1);
const vector3df Editor::CAMERA_MODEL_POS = vector3df(10, 10, -10);
const vector3df Editor::CAMERA_FRONT_POS = vector3df(0, 0, -50);
const vector3df Editor::CAMERA_RIGHT_POS = vector3df(50, 0, 0);

Editor::Editor(Application& application)
    : _application(application),
      _cameraTop(application, CAMERA_TOP_POS, CAMERA_LOOKAT, true),
      _cameraModel(application, CAMERA_MODEL_POS, CAMERA_LOOKAT, false),
      _cameraFront(application, CAMERA_FRONT_POS, CAMERA_LOOKAT, true),
      _cameraRight(application, CAMERA_RIGHT_POS, CAMERA_LOOKAT, true),
      _vTop(_application, _cameraTop),
      _vModel(_application, _cameraModel),
      _vFront(_application, _cameraFront),
      _vRight(_application, _cameraRight),
      _activeViewport(nullptr),
      _vertex(std::make_unique<Mode::Vertex>(_application))
{
    // Set the custom up vector for the top camera
    _cameraTop.SetUpVector(CAMERA_TOP_UP);

    _setViewports();
    _setupDefaultMesh();
}

Editor::~Editor()
{
    std::cout << "Shutdown Editor" << std::endl;
}

void Editor::Draw()
{
    _vTop.RenderWireframe(_defaultMesh);
    _vModel.Render(_defaultMesh);
    _vFront.RenderWireframe(_defaultMesh);
    _vRight.RenderWireframe(_defaultMesh);
    _application.driver->setViewPort(rect<s32>(0, 0, _screenSize.Width, _screenSize.Height));
}

void Editor::Update()
{
    _setViewports();
    _vertex->RemoveHighlight();

    // Get active viewport
    if (_vTop.IsActive(_application.receiver.MouseState.Position)) { _activeViewport = &_vTop; }
    else if (_vModel.IsActive(_application.receiver.MouseState.Position)) { _activeViewport = &_vModel; }
    else if (_vFront.IsActive(_application.receiver.MouseState.Position)) { _activeViewport = &_vFront; }
    else if (_vRight.IsActive(_application.receiver.MouseState.Position)) { _activeViewport = &_vRight; }
    else { _activeViewport = nullptr; }

    // Model rotation
    if (_application.receiver.MouseState.IsDragging && _activeViewport == &_vModel) {
        position2di mouseDelta = _application.receiver.MouseState.Position - _application.receiver.MouseState.LastPosition;
        _activeViewport->GetCamera().Rotate(mouseDelta.X, mouseDelta.Y);
    }

    // Highlight vertex
    if (_activeViewport && 
        _activeViewport != &_vModel) {
        VertexSelection selection = _vertex->Select(_defaultMesh, _activeViewport->GetCamera().GetCameraSceneNode(), _activeViewport->GetViewportSegment());

        if (selection.isSelected) {
            _vertex->AddHighlight(selection);
        }
    }

    // Select vertex
    if (_activeViewport &&
        _activeViewport != &_vModel &&
        _vertex->GetHighlighted() &&
        _application.receiver.MouseState.LeftButtonDown) {
        
        ISceneNode* selected = _application.smgr->addCubeSceneNode(0.5f);
        selected->setPosition(_vertex->GetHighlighted()->getPosition());
        selected->setMaterialFlag(EMF_LIGHTING, true);
        selected->setMaterialFlag(EMF_ZBUFFER, false);
        selected->setMaterialFlag(EMF_ZWRITE_ENABLE, false);  // Don't write to depth buffer

        selected->getMaterial(0).EmissiveColor.set(255, 0, 255, 0); // Bright green

        _vertex->AddToSelected(selected);

        _vertex->RemoveHighlight();
    }

    // Move vertex
    if (_activeViewport &&
        _activeViewport != &_vModel &&
        !_vertex->GetSelected().empty() &&
        _vertex->GetSelected()[0] != nullptr &&
        _application.receiver.MouseState.LeftButtonDown
    ) {
      // Move selected vertices
    }
}

void Editor::ClearVertices()
{
    _vertex->Clear();
}

void Editor::_setupDefaultMesh()
{
    _collisionManager = _application.smgr->getSceneCollisionManager();

    // Scene Setup
    _defaultMesh = _application.smgr->addCubeSceneNode(10.0f);
    if (_defaultMesh) {
        _defaultMesh->setPosition(vector3df(0, 0, 0));
        _defaultMesh->setMaterialFlag(EMF_LIGHTING, false);
    }

    _application.smgr->addLightSceneNode(0, vector3df(0, 20, -20), SColorf(1.0f, 1.0f, 1.0f), 20.0f);
}

void Editor::_setViewports()
{
    _screenSize = _application.driver->getScreenSize();
    s32 w = _screenSize.Width;
    s32 h = _screenSize.Height;
    s32 midW = w / 2;
    s32 midH = h / 2;

    _vTop.UpdateViewport(0, 0, midW, midH);
    _vModel.UpdateViewport(midW, 0, w, midH);
    _vFront.UpdateViewport(0, midH, midW, h);
    _vRight.UpdateViewport(midW, midH, w, h);
}

