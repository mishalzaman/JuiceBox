#pragma once

#include <irrlicht.h>

using namespace irr;
using namespace core;
using namespace gui;

// Forward declare ImGui handler
class ImGuiInputHandler;

class JuiceBoxEventListener : public irr::IEventReceiver
{
public:
    // Mouse State
    struct SMouseState {
        position2di Position;
        bool LeftButtonDown;
    } MouseState;

    bool KeyIsDown[KEY_KEY_CODES_COUNT];

    JuiceBoxEventListener() {
        for (u32 i = 0; i < KEY_KEY_CODES_COUNT; ++i)
            KeyIsDown[i] = false;
        
        MouseState.LeftButtonDown = false;
    }

    virtual bool OnEvent(const SEvent& event) {
        // Forward event to ImGui (declared in main.cpp)
        extern void forwardEventToImGui(const SEvent& event);
        forwardEventToImGui(event);
        
        if (event.EventType == EET_MOUSE_INPUT_EVENT) {
            switch(event.MouseInput.Event) {
                case EMIE_LMOUSE_PRESSED_DOWN:
                    MouseState.LeftButtonDown = true;
                    break;
                case EMIE_LMOUSE_LEFT_UP:
                    MouseState.LeftButtonDown = false;
                    break;
                case EMIE_MOUSE_MOVED:
                    MouseState.Position.X = event.MouseInput.X;
                    MouseState.Position.Y = event.MouseInput.Y;
                    break;
                default:
                    break;
            }
        }
        
        else if (event.EventType == EET_KEY_INPUT_EVENT) {
            KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;
            return false;
        }
        
        return false;
    }

    virtual bool IsKeyDown(EKEY_CODE keyCode) const {
        return KeyIsDown[keyCode];
    }
};