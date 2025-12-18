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
