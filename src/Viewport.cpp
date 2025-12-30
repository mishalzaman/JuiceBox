#include "Viewport.h"

Viewport::Viewport(Application& application, Camera& camera, ViewportType viewportType)
    :_application(application),
    _camera(camera),
    _viewPortType(viewportType),
    _renderTexture(nullptr)
{
}

Viewport::~Viewport()
{
    if (_renderTexture) {
        _application.driver->removeTexture(_renderTexture);
    }
}

void Viewport::UpdateViewport(s32 top_left_x, s32 top_left_y, s32 bottom_right_x, s32 bottom_right_y)
{
    this->_viewportSegment = rect<s32>(top_left_x, top_left_y, bottom_right_x, bottom_right_y);
    _createRenderTexture();
}

void Viewport::_createRenderTexture()
{
    // Calculate viewport dimensions
    s32 viewportWidth = _viewportSegment.getWidth();
    s32 viewportHeight = _viewportSegment.getHeight();
    
    // Calculate render texture dimensions (max 640 width, maintain aspect ratio)
    s32 renderWidth = viewportWidth;
    s32 renderHeight = viewportHeight;
    
    if (renderWidth > MAX_RENDER_WIDTH) {
        f32 scale = (f32)MAX_RENDER_WIDTH / (f32)viewportWidth;
        renderWidth = MAX_RENDER_WIDTH;
        renderHeight = (s32)(viewportHeight * scale);
    }
    
    // Remove old texture if it exists
    if (_renderTexture) {
        _application.driver->removeTexture(_renderTexture);
    }
    
    // Create new render target texture
    _renderTexture = _application.driver->addRenderTargetTexture(
        dimension2d<u32>(renderWidth, renderHeight),
        "rt",
        ECF_A8R8G8B8
    );
}

void Viewport::_renderToTexture(IMeshSceneNode* mesh, bool wireframe)
{
    if (!_renderTexture) return;
    
    // Set render target to our texture
    _application.driver->setRenderTarget(_renderTexture, true, true, SColor(255, 100, 100, 100));
    
    // Set camera and material flags
    _application.smgr->setActiveCamera(_camera.GetCameraSceneNode());
    
    if (mesh) {
        mesh->setMaterialFlag(EMF_WIREFRAME, wireframe);
        mesh->setMaterialFlag(EMF_LIGHTING, !wireframe);
        mesh->setMaterialFlag(EMF_BILINEAR_FILTER, false);
        mesh->setMaterialFlag(EMF_ANISOTROPIC_FILTER, false);
    }
    
    // Render the scene
    _application.smgr->drawAll();
    
    // Reset render target to screen
    _application.driver->setRenderTarget(0, false, false);
}

void Viewport::_drawTextureToViewport()
{
    if (!_renderTexture) return;
    
    // Reset viewport to full screen for 2D drawing
    dimension2d<u32> screenSize = _application.driver->getScreenSize();
    _application.driver->setViewPort(rect<s32>(0, 0, screenSize.Width, screenSize.Height));
    
    // Calculate the destination rectangle using absolute screen coordinates
    rect<s32> destRect(
        _viewportSegment.UpperLeftCorner.X,
        _viewportSegment.UpperLeftCorner.Y,
        _viewportSegment.LowerRightCorner.X,
        _viewportSegment.LowerRightCorner.Y
    );
    
    // Source rectangle (entire render texture)
    rect<s32> sourceRect(
        0, 0,
        _renderTexture->getSize().Width,
        _renderTexture->getSize().Height
    );
    
    // Draw the render texture stretched to fill the viewport
    _application.driver->draw2DImage(
        _renderTexture,
        destRect,
        sourceRect,
        nullptr,
        nullptr,
        true
    );
}

void Viewport::Render(IMeshSceneNode* mesh)
{
    _renderToTexture(mesh, false);
    _drawTextureToViewport();
}

void Viewport::RenderWireframe(IMeshSceneNode *mesh)
{
    _renderToTexture(mesh, true);
    _drawTextureToViewport();
}

bool Viewport::IsActive(position2di mousePosition)
{
    return _viewportSegment.isPointInside(mousePosition);
}