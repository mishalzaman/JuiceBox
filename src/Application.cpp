#include "Application.h"
#include <iostream>

Application::Application() : device(nullptr), driver(nullptr), smgr(nullptr) {
}

Application::~Application() {
    if (device) {
        device->drop();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();

    std::cout << "Shutdown Application" << std::endl;
}

bool Application::BeginCore() {
    this->_windowResolution = WindowResolution::Get();

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
    driver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);
    smgr = device->getSceneManager();

    return true;
}

void Application::BeginGUI() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 130");
}
