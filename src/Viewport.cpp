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

void Viewport::Render()
{
}
