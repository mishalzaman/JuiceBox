#pragma once

#include <irrlicht.h>

using namespace irr;
using namespace core;
using namespace gui;

class JuiceBoxEventListener : public irr::IEventReceiver
{
public:
    struct SMouseState {
        position2di Position;      // Current mouse position
        position2di LastPosition;  // Position in the previous frame
        position2di ClickPosition; // Where the mouse was when button was first pressed
        bool LeftButtonDown;
        bool IsDragging;

        // Number of pixels to move before a "click" becomes a "drag"
        const s32 DragThreshold = 4; 
    } MouseState;

    bool KeyIsDown[KEY_KEY_CODES_COUNT];

    JuiceBoxEventListener() {
        for (u32 i = 0; i < KEY_KEY_CODES_COUNT; ++i)
            KeyIsDown[i] = false;
        
        MouseState.LeftButtonDown = false;
        MouseState.IsDragging = false;
        MouseState.Position = position2di(0, 0);
        MouseState.LastPosition = position2di(0, 0);
        MouseState.ClickPosition = position2di(0, 0);
    }

    virtual bool OnEvent(const SEvent& event) {
        // Forward event to ImGui
        extern void forwardEventToImGui(const SEvent& event);
        forwardEventToImGui(event);
        
        if (event.EventType == EET_MOUSE_INPUT_EVENT) {
            switch(event.MouseInput.Event) {
                case EMIE_LMOUSE_PRESSED_DOWN:
                    MouseState.LeftButtonDown = true;
                    MouseState.IsDragging = false; // Reset dragging state
                    MouseState.Position = position2di(event.MouseInput.X, event.MouseInput.Y);
                    MouseState.ClickPosition = MouseState.Position;
                    MouseState.LastPosition = MouseState.Position;
                    break;

                case EMIE_LMOUSE_LEFT_UP:
                    MouseState.LeftButtonDown = false;
                    MouseState.IsDragging = false;
                    break;

                case EMIE_MOUSE_MOVED:
                    MouseState.LastPosition = MouseState.Position;
                    MouseState.Position.X = event.MouseInput.X;
                    MouseState.Position.Y = event.MouseInput.Y;

                    // If button is held, check if we should start dragging
                    if (MouseState.LeftButtonDown && !MouseState.IsDragging) {
                        // Calculate distance from initial click
                        s32 distance = MouseState.Position.getDistanceFrom(MouseState.ClickPosition);
                        if (distance >= MouseState.DragThreshold) {
                            MouseState.IsDragging = true;
                        }
                    }
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

    // Helper to check key state
    virtual bool IsKeyDown(EKEY_CODE keyCode) const {
        return KeyIsDown[keyCode];
    }

    core::vector2di GetMouseDelta() const {
        return MouseState.Position - MouseState.LastPosition;
    }

    void UpdateLastPosition() {
        MouseState.LastPosition = MouseState.Position;
    }
};