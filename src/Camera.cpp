#include "Camera.h"

Camera::Camera(
        Application& application,
        vector3df position,
        vector3df lookat,
        bool isOrtho
    ):
    _position(position), _lookat(lookat)
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
}

Camera::~Camera()
{
}

void Camera::Rotate(float mouseDeltaX, float mouseDeltaY)
{
    _theta -= mouseDeltaX * 0.2f; 
    _phi += mouseDeltaY * 0.2f;  
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
