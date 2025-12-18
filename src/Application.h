#pragma once

#include <irrlicht.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "JuiceBoxEventListener.h"
#include "helpers/WindowResolution.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class Application {
public:
    // Pointers to core components
    IrrlichtDevice* device;
    IVideoDriver* driver;
    ISceneManager* smgr;
    JuiceBoxEventListener receiver;

    Application(); // Constructor declaration
    ~Application(); // Destructor declaration

    bool BeginCore();
    void BeginGUI();
private:
    dimension2d<u32> _windowResolution;
};