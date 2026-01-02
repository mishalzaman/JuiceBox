#pragma once

#include "imgui.h" // Ensure this is included

class UITopBar {
public:
    UITopBar();
    ~UITopBar();

    // Call this ONCE after ImGui::CreateContext() but before the loop
    void Init(); 
    
    // Call this every frame
    void Render();

private:
    static const int MAX_HEIGHT = 64;
    void FileBtn();
    void OptionsBtn();
    void AboutBtn();

    ImFont* _font = nullptr;
};