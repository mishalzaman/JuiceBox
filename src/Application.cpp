#include "Application.h"

// For standard output/debugging (optional)
#include <iostream>

// --- Constructor ---
Application::Application() : device(nullptr), driver(nullptr), smgr(nullptr) {
    // Member initializers handle the initial null values
    this->_initialize();
}

// --- Destructor ---
Application::~Application() {
    if (device) {
        device->drop();
    }
}

// --- GetDesktopResolution Definition ---
void Application::_setWindowResolution() {
    // 1. Create a temporary NULL device to get the video mode list
    IrrlichtDevice *nullDevice = createDevice(EDT_NULL);
    
    // Check if device creation failed
    if (!nullDevice){ 
        this->_windowResolution = dimension2d<u32>(1024, 768); // Fallback
    }

    IVideoModeList* modeList = nullDevice->getVideoModeList();

    if (modeList)
    {
        this->_windowResolution = modeList->getDesktopResolution();
    }
    else
    {
        // Fallback resolution
        this->_windowResolution = dimension2d<u32>(1024, 768);
    }
    
    // Crucially, delete the temporary NULL device before creating the real device
    nullDevice->drop();
}

// --- Initialize Definition ---
bool Application::_initialize() {
    _setWindowResolution();

    // 4. Create the main device using the fetched resolution
    device = createDevice(
        EDT_OPENGL,                 // The video driver type.
        this->_windowResolution,    // Desktop resolution
        32,                         // Color depth
        false,                       // Set to true for full-screen for a typical setup.
        false,                      // Stencil buffer off.
        false,                      // V-Sync off.
        &receiver                   // Pointer to the custom IEventReceiver.
    );

    if (!device)
        return false;

    device->setWindowCaption(L"Juice Box v0.1.0");

    driver = device->getVideoDriver();
    smgr = device->getSceneManager();

    return true;
}