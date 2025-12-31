#pragma once

#include "Application.h"
#include "Camera.h"
#include "Types.h"

class Viewport {
    public:
        Viewport(Application& application, Camera& camera, ViewportType viewportType);
        ~Viewport();

        void UpdateViewport(s32 top_left_x, s32 top_left_y, s32 bottom_right_x, s32 bottom_right_y);
        void Render(IMeshSceneNode* mesh);
        void RenderWireframe(IMeshSceneNode* mesh);
        bool IsActive(position2di mousePosition);
        Camera& GetCamera() { return _camera; }
        rect<s32> GetViewportSegment() { return _viewportSegment; }
        ViewportType GetViewportType() { return _viewPortType; }

    private:
        Application& _application;
        Camera& _camera;

        rect<s32> _viewportSegment;
        ViewportType _viewPortType;
        
        // Render texture
        ITexture* _renderTexture;
        static constexpr s32 MAX_RENDER_WIDTH = 640/2;
        
        void _createRenderTexture();
        void _renderToTexture(IMeshSceneNode* mesh, bool wireframe);
        void _drawTextureToViewport();
};