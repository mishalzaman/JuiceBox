#pragma once

#include <irrlicht.h>

using namespace irr;
using namespace core;
using namespace video;

namespace WindowResolution {
    inline dimension2d<u32> Get() {
        dimension2d<u32> resolution(1024, 768); // Default fallback

        // 1. Create a temporary NULL device
        IrrlichtDevice *nullDevice = createDevice(EDT_NULL);
        
        if (nullDevice) {
            IVideoModeList* modeList = nullDevice->getVideoModeList();
            if (modeList) {
                resolution = modeList->getDesktopResolution();
            }
            
            // 2. Drop the null device to free memory
            nullDevice->drop();
        }

        return resolution;
    }
}
