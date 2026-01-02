#include "UITopBar.h"
#include <iostream>

UITopBar::UITopBar()
{
    // DO NOT load fonts here. 
    // This runs before ImGui::CreateContext() and causes the crash.
}

UITopBar::~UITopBar()
{
}

void UITopBar::Init() {
    // This is called explicitly from main() after ImGui is setup
    ImGuiIO& io = ImGui::GetIO();
    
    // Load the font once. 
    _font = io.Fonts->AddFontFromFileTTF("assets/bigblueterminalregular.ttf", 24.0f);

    if (_font == nullptr) {
        std::cerr << "Failed to load font: assets/bigblueterminalregular.ttf" << std::endl;
    }
}

void UITopBar::FileBtn() {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.75f, 0.75f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.5f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    
    // Explicitly push the font if it exists to ensure buttons use it
    if (_font) ImGui::PushFont(_font);

    if (ImGui::Button("File", ImVec2(100, 48))) {
        // Functionality to be added later
        std::cout << "File Clicked" << std::endl;
    }
    
    if (_font) ImGui::PopFont();

    ImGui::PopStyleColor(4);
}

void UITopBar::OptionsBtn() {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.75f, 0.75f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.5f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    
    if (_font) ImGui::PushFont(_font);

    if (ImGui::Button("Options", ImVec2(100, 48))) {
        // Functionality to be added later
        std::cout << "Options Clicked" << std::endl;
    }
    
    if (_font) ImGui::PopFont();

    ImGui::PopStyleColor(4);
}

void UITopBar::AboutBtn() {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.75f, 0.75f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.5f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    
    if (_font) ImGui::PushFont(_font);

    if (ImGui::Button("About", ImVec2(100, 48))) {
        // Functionality to be added later
        std::cout << "About Clicked" << std::endl;
    }
    
    if (_font) ImGui::PopFont();

    ImGui::PopStyleColor(4);
}

void UITopBar::Render() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(16, 4));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.75f, 0.75f, 0.75f, 1.0f));
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | 
                                    ImGuiWindowFlags_NoResize | 
                                    ImGuiWindowFlags_NoMove | 
                                    ImGuiWindowFlags_NoScrollbar |
                                    ImGuiWindowFlags_NoCollapse;
    
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, MAX_HEIGHT));
    
    ImGui::Begin("TopBar", nullptr, window_flags);
    
    FileBtn();
    ImGui::SameLine();
    OptionsBtn();
    ImGui::SameLine();
    AboutBtn(); // Fixed: Added ();
    
    ImGui::End();
    
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}