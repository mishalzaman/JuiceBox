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
      _vTop(_application, _cameraTop, ViewportType::TOP),
      _vModel(_application, _cameraModel, ViewportType::MODEL),
      _vFront(_application, _cameraFront, ViewportType::FRONT),
      _vRight(_application, _cameraRight, ViewportType::RIGHT),
      _activeViewport(nullptr),
      _model(std::make_unique<Model>(_application)),
      _mode(EditorMode::VERTEX)
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
    _setActiveViewport();
    _model->ClearHighlighted();

    // Model rotation
    if (_application.receiver.MouseState.IsDragging && _activeViewport == &_vModel) {
        position2di mouseDelta = _application.receiver.MouseState.Position - _application.receiver.MouseState.LastPosition;
        _activeViewport->GetCamera().Rotate(mouseDelta.X, mouseDelta.Y);
    }

    // Highlight vertex
    if (_activeViewport && 
        _activeViewport != &_vModel &&
        !_application.receiver.MouseState.IsDragging) {
        VertexSelection selection = UVertex::Select(
            _defaultMesh,
            _activeViewport->GetCamera().GetCameraSceneNode(),
            _activeViewport->GetViewportSegment(),
            _application.receiver.MouseState.Position,
            _mode
        );

        if (selection.isSelected) {
            _model->SetHighlightedVertex(selection.worldPos);
        }
    }

    // Select vertex
    if (_activeViewport &&
        _activeViewport != &_vModel &&
        _model->HasHighlightedVertex() &&
        _application.receiver.MouseState.LeftButtonDown) {
        
        _model->AddSelectedVertex();
    }

    // Move vertex (UPDATED LOGIC)
    if (_activeViewport &&          
        _activeViewport != &_vModel &&
        !_model->GetSelectedVertices().empty() &&
        _application.receiver.MouseState.LeftButtonDown
    ) {
        ISceneNode* pivotNode = _model->GetSelectedVertices()[0];
        
        if (pivotNode) {
            vector3df pivotPos = pivotNode->getPosition();

            // A. Calculate World Position for CURRENT mouse frame
            vector3df currentWorldPos = UVertex::Move(
                _collisionManager,
                _activeViewport->GetCamera().GetCameraSceneNode(),
                _application.receiver.MouseState.Position,
                pivotPos, 
                _activeViewport->GetViewportType(),
                _activeViewport->GetViewportSegment()
            );

            // B. Calculate World Position for PREVIOUS mouse frame
            // We use the same pivotPos to ensure we are projecting onto the same depth plane
            vector3df lastWorldPos = UVertex::Move(
                _collisionManager,
                _activeViewport->GetCamera().GetCameraSceneNode(),
                _application.receiver.MouseState.LastPosition,
                pivotPos, 
                _activeViewport->GetViewportType(),
                _activeViewport->GetViewportSegment()
            );

            // C. The difference is the actual 3D movement vector for this frame
            vector3df moveDelta = currentWorldPos - lastWorldPos;

            // Only update if there is actual movement
            if (moveDelta.getLengthSQ() > 0.000001f) {
                for (ISceneNode* selectedNode : _model->GetSelectedVertices()) {
                    if (selectedNode) {
                        vector3df oldPos = selectedNode->getPosition();
                        vector3df newPos = oldPos + moveDelta;

                        _model->UpdateMesh(oldPos, newPos);
                        selectedNode->setPosition(newPos);
                    }
                }
            }
        }
    }
}

void Editor::ClearVertices()
{
    _model->ClearAll();
}

void Editor::_setupDefaultMesh()
{
    _model->GenerateDefault();
    _defaultMesh = _model->GetMesh();
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

void Editor::_setActiveViewport()
{
    _activeViewport = nullptr;

    if (_vTop.IsActive(_application.receiver.MouseState.Position)) { 
        _activeViewport = &_vTop; 
    }
    else if (_vModel.IsActive(_application.receiver.MouseState.Position)) { 
        _activeViewport = &_vModel; 
    }
    else if (_vFront.IsActive(_application.receiver.MouseState.Position)) { 
        _activeViewport = &_vFront; 
    }
    else if (_vRight.IsActive(_application.receiver.MouseState.Position)) { 
        _activeViewport = &_vRight; 
    }
}
