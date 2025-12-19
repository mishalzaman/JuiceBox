#pragma once

#include "Application.h"
#include "Camera.h"

class Viewport {
    public:
        Viewport(Application& application, Camera& camera);
        ~Viewport();

        void UpdateViewport(s32 top_left_x, s32 top_left_y, s32 bottom_right_x, s32 bottom_right_y);
        void Render(IMeshSceneNode* mesh);
        void RenderWireframe(IMeshSceneNode* mesh);
        bool IsActive(position2di mousePosition);

    private:
        Application& _application;
        Camera& _camera;

        rect<s32> _viewportSegment;
};