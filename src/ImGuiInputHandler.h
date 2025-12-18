#include <irrlicht.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class ImGuiInputHandler {
public:
    static bool wantCaptureMouse;
    static bool wantCaptureKeyboard;
    
    static void processEvent(const SEvent& event) {
        ImGuiIO& io = ImGui::GetIO();
        
        if (event.EventType == EET_MOUSE_INPUT_EVENT) {
            io.MousePos = ImVec2((float)event.MouseInput.X, (float)event.MouseInput.Y);
            
            if (event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN)
                io.MouseDown[0] = true;
            if (event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP)
                io.MouseDown[0] = false;
            if (event.MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN)
                io.MouseDown[1] = true;
            if (event.MouseInput.Event == EMIE_RMOUSE_LEFT_UP)
                io.MouseDown[1] = false;
            if (event.MouseInput.Event == EMIE_MMOUSE_PRESSED_DOWN)
                io.MouseDown[2] = true;
            if (event.MouseInput.Event == EMIE_MMOUSE_LEFT_UP)
                io.MouseDown[2] = false;
            if (event.MouseInput.Event == EMIE_MOUSE_WHEEL)
                io.MouseWheel += event.MouseInput.Wheel;
        }
        
        if (event.EventType == EET_KEY_INPUT_EVENT) {
            // Casting the Irrlicht key directly to ImGuiKey for now to resolve error
            io.AddKeyEvent((ImGuiKey)event.KeyInput.Key, event.KeyInput.PressedDown);
            
            if (event.KeyInput.PressedDown && event.KeyInput.Char != 0) {
                io.AddInputCharacter(event.KeyInput.Char);
            }
        }
        
        wantCaptureMouse = io.WantCaptureMouse;
        wantCaptureKeyboard = io.WantCaptureKeyboard;
    }
};