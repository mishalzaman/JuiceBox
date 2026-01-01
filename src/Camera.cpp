#include "Camera.h"

Camera::Camera(
        Application& application,
        vector3df position,
        vector3df lookat,
        bool isOrtho
    ):
    _position(position),
    _lookat(lookat),
    _application(application)
{
    _camera = application.smgr->addCameraSceneNode(0, _position, _lookat);

    if (isOrtho) {
        _orthographic.buildProjectionMatrixOrthoLH(
            ORTHO_WIDTH_VIEW,
            ORTHO_HEIGHT_VIEW,
            ORTHO_NEAR,
            ORTHO_FAR
        );
        _camera->setProjectionMatrix(_orthographic, true);
    }
    _setInitialPosition();
}

Camera::~Camera()
{
}

void Camera::Rotate()
{
    if (!_application.receiver.MouseState.IsDragging) {
        return;
    }

    position2di mouseDelta = _application.receiver.MouseState.Position - _application.receiver.MouseState.LastPosition;
    _theta -= mouseDelta.X * _sensitivity; 
    _phi += mouseDelta.Y * _sensitivity;  
    if (_phi > 89.0f) _phi = 89.0f;
    if (_phi < -89.0f) _phi = -89.0f;
    f32 r = _cameraRadius;
    f32 radTheta = _theta * DEGTORAD;
    f32 radPhi = _phi * DEGTORAD;
    f32 x = r * cos(radPhi) * sin(radTheta);
    f32 y = r * sin(radPhi);
    f32 z = r * cos(radPhi) * cos(radTheta) * -1; 
    _camera->setPosition(vector3df(x, y, z));
    _camera->setTarget(vector3df(0,0,0)); 
}

void Camera::_setInitialPosition()
{
    vector3df pos = _camera->getPosition();
    _cameraRadius = pos.getLength();

    // Calculate Phi (Latitude) - angle from the XZ plane
    // asin returns radians, convert to degrees
    _phi = asin(pos.Y / _cameraRadius) * RADTODEG;

    // Calculate Theta (Longitude)
    // atan2 handles the quadrants correctly
    // We multiply z by -1 to match your specific coordinate mapping in Rotate()
    _theta = atan2(pos.X, pos.Z * -1.0f) * RADTODEG;

    // Clamp phi just in case the initial position is exactly top/bottom
    if (_phi > 89.0f) _phi = 89.0f;
    if (_phi < -89.0f) _phi = -89.0f;
}