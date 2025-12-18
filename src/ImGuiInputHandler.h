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
    
    // Map Irrlicht keys to ImGui keys
    static ImGuiKey IrrlichtKeyToImGuiKey(EKEY_CODE key) {
        switch (key) {
            case KEY_TAB: return ImGuiKey_Tab;
            case KEY_LEFT: return ImGuiKey_LeftArrow;
            case KEY_RIGHT: return ImGuiKey_RightArrow;
            case KEY_UP: return ImGuiKey_UpArrow;
            case KEY_DOWN: return ImGuiKey_DownArrow;
            case KEY_PRIOR: return ImGuiKey_PageUp;
            case KEY_NEXT: return ImGuiKey_PageDown;
            case KEY_HOME: return ImGuiKey_Home;
            case KEY_END: return ImGuiKey_End;
            case KEY_INSERT: return ImGuiKey_Insert;
            case KEY_DELETE: return ImGuiKey_Delete;
            case KEY_BACK: return ImGuiKey_Backspace;
            case KEY_SPACE: return ImGuiKey_Space;
            case KEY_RETURN: return ImGuiKey_Enter;
            case KEY_ESCAPE: return ImGuiKey_Escape;
            case KEY_OEM_7: return ImGuiKey_Apostrophe;
            case KEY_COMMA: return ImGuiKey_Comma;
            case KEY_MINUS: return ImGuiKey_Minus;
            case KEY_PERIOD: return ImGuiKey_Period;
            case KEY_OEM_2: return ImGuiKey_Slash;
            case KEY_OEM_1: return ImGuiKey_Semicolon;
            case KEY_PLUS: return ImGuiKey_Equal;
            case KEY_OEM_4: return ImGuiKey_LeftBracket;
            case KEY_OEM_5: return ImGuiKey_Backslash;
            case KEY_OEM_6: return ImGuiKey_RightBracket;
            case KEY_OEM_3: return ImGuiKey_GraveAccent;
            case KEY_CAPITAL: return ImGuiKey_CapsLock;
            case KEY_SCROLL: return ImGuiKey_ScrollLock;
            case KEY_NUMLOCK: return ImGuiKey_NumLock;
            case KEY_SNAPSHOT: return ImGuiKey_PrintScreen;
            case KEY_PAUSE: return ImGuiKey_Pause;
            
            // Numpad
            case KEY_NUMPAD0: return ImGuiKey_Keypad0;
            case KEY_NUMPAD1: return ImGuiKey_Keypad1;
            case KEY_NUMPAD2: return ImGuiKey_Keypad2;
            case KEY_NUMPAD3: return ImGuiKey_Keypad3;
            case KEY_NUMPAD4: return ImGuiKey_Keypad4;
            case KEY_NUMPAD5: return ImGuiKey_Keypad5;
            case KEY_NUMPAD6: return ImGuiKey_Keypad6;
            case KEY_NUMPAD7: return ImGuiKey_Keypad7;
            case KEY_NUMPAD8: return ImGuiKey_Keypad8;
            case KEY_NUMPAD9: return ImGuiKey_Keypad9;
            case KEY_DECIMAL: return ImGuiKey_KeypadDecimal;
            case KEY_DIVIDE: return ImGuiKey_KeypadDivide;
            case KEY_MULTIPLY: return ImGuiKey_KeypadMultiply;
            case KEY_SUBTRACT: return ImGuiKey_KeypadSubtract;
            case KEY_ADD: return ImGuiKey_KeypadAdd;
            
            // Modifiers
            case KEY_LSHIFT: return ImGuiKey_LeftShift;
            case KEY_LCONTROL: return ImGuiKey_LeftCtrl;
            case KEY_LMENU: return ImGuiKey_LeftAlt;
            case KEY_LWIN: return ImGuiKey_LeftSuper;
            case KEY_RSHIFT: return ImGuiKey_RightShift;
            case KEY_RCONTROL: return ImGuiKey_RightCtrl;
            case KEY_RMENU: return ImGuiKey_RightAlt;
            case KEY_RWIN: return ImGuiKey_RightSuper;
            case KEY_MENU: return ImGuiKey_Menu;
            
            // Numbers
            case KEY_KEY_0: return ImGuiKey_0;
            case KEY_KEY_1: return ImGuiKey_1;
            case KEY_KEY_2: return ImGuiKey_2;
            case KEY_KEY_3: return ImGuiKey_3;
            case KEY_KEY_4: return ImGuiKey_4;
            case KEY_KEY_5: return ImGuiKey_5;
            case KEY_KEY_6: return ImGuiKey_6;
            case KEY_KEY_7: return ImGuiKey_7;
            case KEY_KEY_8: return ImGuiKey_8;
            case KEY_KEY_9: return ImGuiKey_9;
            
            // Letters
            case KEY_KEY_A: return ImGuiKey_A;
            case KEY_KEY_B: return ImGuiKey_B;
            case KEY_KEY_C: return ImGuiKey_C;
            case KEY_KEY_D: return ImGuiKey_D;
            case KEY_KEY_E: return ImGuiKey_E;
            case KEY_KEY_F: return ImGuiKey_F;
            case KEY_KEY_G: return ImGuiKey_G;
            case KEY_KEY_H: return ImGuiKey_H;
            case KEY_KEY_I: return ImGuiKey_I;
            case KEY_KEY_J: return ImGuiKey_J;
            case KEY_KEY_K: return ImGuiKey_K;
            case KEY_KEY_L: return ImGuiKey_L;
            case KEY_KEY_M: return ImGuiKey_M;
            case KEY_KEY_N: return ImGuiKey_N;
            case KEY_KEY_O: return ImGuiKey_O;
            case KEY_KEY_P: return ImGuiKey_P;
            case KEY_KEY_Q: return ImGuiKey_Q;
            case KEY_KEY_R: return ImGuiKey_R;
            case KEY_KEY_S: return ImGuiKey_S;
            case KEY_KEY_T: return ImGuiKey_T;
            case KEY_KEY_U: return ImGuiKey_U;
            case KEY_KEY_V: return ImGuiKey_V;
            case KEY_KEY_W: return ImGuiKey_W;
            case KEY_KEY_X: return ImGuiKey_X;
            case KEY_KEY_Y: return ImGuiKey_Y;
            case KEY_KEY_Z: return ImGuiKey_Z;
            
            // Function keys
            case KEY_F1: return ImGuiKey_F1;
            case KEY_F2: return ImGuiKey_F2;
            case KEY_F3: return ImGuiKey_F3;
            case KEY_F4: return ImGuiKey_F4;
            case KEY_F5: return ImGuiKey_F5;
            case KEY_F6: return ImGuiKey_F6;
            case KEY_F7: return ImGuiKey_F7;
            case KEY_F8: return ImGuiKey_F8;
            case KEY_F9: return ImGuiKey_F9;
            case KEY_F10: return ImGuiKey_F10;
            case KEY_F11: return ImGuiKey_F11;
            case KEY_F12: return ImGuiKey_F12;
            
            default: return ImGuiKey_None;
        }
    }
    
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
            // Convert Irrlicht key to ImGui key
            ImGuiKey imguiKey = IrrlichtKeyToImGuiKey(event.KeyInput.Key);
            
            // Only send the event if we have a valid mapping
            if (imguiKey != ImGuiKey_None) {
                io.AddKeyEvent(imguiKey, event.KeyInput.PressedDown);
            }
            
            // Handle text input separately
            if (event.KeyInput.PressedDown && event.KeyInput.Char != 0) {
                io.AddInputCharacter(event.KeyInput.Char);
            }
            
            // Handle modifier keys
            io.AddKeyEvent(ImGuiMod_Ctrl, event.KeyInput.Control);
            io.AddKeyEvent(ImGuiMod_Shift, event.KeyInput.Shift);
        }
        
        wantCaptureMouse = io.WantCaptureMouse;
        wantCaptureKeyboard = io.WantCaptureKeyboard;
    }
};