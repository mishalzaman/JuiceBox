#pragma once

#include <irrlicht.h>
#include "JuiceBoxEventListener.h" // Assuming this is needed for the receiver setup

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
private:
    bool _initialize();
    void _setWindowResolution();

    dimension2d<u32> _windowResolution;
};