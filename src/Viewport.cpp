#include "Viewport.h"

Viewport::Viewport(Application& application, Camera& camera)
    :_application(application),
    _camera(camera)
{
}

Viewport::~Viewport()
{
}

void Viewport::UpdateViewport(s32 top_left_x, s32 top_left_y, s32 bottom_right_x, s32 bottom_right_y)
{
    this->_viewportSegment = rect<s32>(top_left_x, top_left_y, bottom_right_x, bottom_right_y);
}

void Viewport::Render(IMeshSceneNode* mesh)
{
    _application.driver->setViewPort(_viewportSegment);
    _application.smgr->setActiveCamera(_camera.GetCameraSceneNode());
    if(mesh) mesh->setMaterialFlag(EMF_WIREFRAME, false);
    if(mesh) mesh->setMaterialFlag(EMF_LIGHTING, true);
    _application.smgr->drawAll();
}

void Viewport::RenderWireframe(IMeshSceneNode *mesh)
{
    _application.driver->setViewPort(_viewportSegment);
    _application.smgr->setActiveCamera(_camera.GetCameraSceneNode());
    if(mesh) mesh->setMaterialFlag(EMF_WIREFRAME, true);
    if(mesh) mesh->setMaterialFlag(EMF_LIGHTING, false);
    _application.smgr->drawAll();
}

bool Viewport::IsActive(position2di mousePosition)
{
    return _viewportSegment.isPointInside(mousePosition);
}
