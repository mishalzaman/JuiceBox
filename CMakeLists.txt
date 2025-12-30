cmake_minimum_required(VERSION 3.14)
project(JuiceBox LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ==============================================================================
# 1. Fetch Irrlicht (Existing)
# ==============================================================================
FetchContent_Declare(
    irrlicht
    GIT_REPOSITORY https://github.com/mishalzaman/irrlicht-1.8.5.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(irrlicht)

set(IRRLICHT_ROOT "${irrlicht_SOURCE_DIR}")
set(IRRLICHT_INCLUDE_DIR "${IRRLICHT_ROOT}/include")

# ==============================================================================
# 2. Fetch Dear ImGui
# ==============================================================================
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.92.5
)
FetchContent_MakeAvailable(imgui)

# Define ImGui Source Files (without non-existent Irrlicht backend)
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(ImGui STATIC ${IMGUI_SOURCES})
target_include_directories(ImGui PUBLIC 
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# ==============================================================================
# 3. Compile Irrlicht (Linux/Windows Logic)
# ==============================================================================
if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    find_package(OpenGL REQUIRED)
    set(IRRLICHT_LIBRARY_FILE "${IRRLICHT_ROOT}/lib/Linux/libIrrlicht.a")

    add_custom_command(
        OUTPUT "${IRRLICHT_LIBRARY_FILE}"
        COMMAND make NDEBUG=1
        WORKING_DIRECTORY "${IRRLICHT_ROOT}/source/Irrlicht"
    )
    add_custom_target(IrrlichtMake DEPENDS "${IRRLICHT_LIBRARY_FILE}")
    add_library(Irrlicht STATIC IMPORTED GLOBAL)
    add_dependencies(Irrlicht IrrlichtMake)
    set_target_properties(Irrlicht PROPERTIES IMPORTED_LOCATION "${IRRLICHT_LIBRARY_FILE}")
    
    # Link OpenGL to ImGui
    target_link_libraries(ImGui PUBLIC ${OPENGL_LIBRARIES})
elseif(WIN32)
    find_package(OpenGL REQUIRED)
    set(IRRLICHT_LIBRARY_FILE "${IRRLICHT_ROOT}/lib/Win64-visualStudio/Irrlicht.lib")
    add_library(Irrlicht STATIC IMPORTED GLOBAL)
    set_target_properties(Irrlicht PROPERTIES IMPORTED_LOCATION "${IRRLICHT_LIBRARY_FILE}")
    
    # Link OpenGL to ImGui
    target_link_libraries(ImGui PUBLIC OpenGL32)
endif()

# ==============================================================================
# 4. Define Main Application
# ==============================================================================
set(JUICEBOX_SOURCES 
    src/main.cpp 
    src/Application.cpp
    src/editor/Editor.cpp
    src/painter/Painter.cpp
    src/Viewport.cpp
    src/Camera.cpp
    src/Model.cpp
)
set(JUICEBOX_HEADERS 
    src/JuiceBoxEventListener.h 
    src/Application.h 
    src/ImGuiInputHandler.h
    src/editor/Editor.h
    src/painter/Painter.h
    src/helpers/WindowResolution.h
    src/helpers/Mesh.h
    src/Viewport.h
    src/Camera.h
    src/Model.h
    src/utility/UVertex.h
    src/Types.h
)

add_executable(JuiceBox ${JUICEBOX_SOURCES})
target_sources(JuiceBox PRIVATE ${JUICEBOX_HEADERS})

# Link libraries
target_link_libraries(JuiceBox PRIVATE Irrlicht ImGui)

if(UNIX AND NOT APPLE)
    target_link_libraries(JuiceBox PRIVATE ${X11_LIBRARIES} ${X11_Xxf86vm_LIB} ${OPENGL_LIBRARIES} dl)
elseif(WIN32)
    target_link_libraries(JuiceBox PRIVATE OpenGL32)
endif()

target_include_directories(JuiceBox PRIVATE 
    ${IRRLICHT_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# ==============================================================================
# 5. Copy Assets Folder to Build Directory
# ==============================================================================
add_custom_command(TARGET JuiceBox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:JuiceBox>/assets
    COMMENT "Copying assets folder to build directory"
)