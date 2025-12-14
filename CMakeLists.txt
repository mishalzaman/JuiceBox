cmake_minimum_required(VERSION 3.14)
project(JuiceBox LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Include FetchContent Module ---
include(FetchContent)

# ==============================================================================
# 1. Fetch Irrlicht (Source Code)
# ==============================================================================
FetchContent_Declare(
    irrlicht
    GIT_REPOSITORY https://github.com/mishalzaman/irrlicht-1.8.5.git
    GIT_TAG        main
)

FetchContent_GetProperties(irrlicht)
if(NOT irrlicht_POPULATED)
    FetchContent_Populate(irrlicht)
endif()

# Set root paths
set(IRRLICHT_ROOT "${irrlicht_SOURCE_DIR}")
set(IRRLICHT_INCLUDE_DIR "${IRRLICHT_ROOT}/include")

# ==============================================================================
# 2. Compile Irrlicht 
# ==============================================================================
# We need to find X11 and OpenGL to compile Irrlicht on Linux
if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    find_package(OpenGL REQUIRED)
    
    # Define where the compiled file WILL be after we build it
    set(IRRLICHT_LIBRARY_FILE "${IRRLICHT_ROOT}/lib/Linux/libIrrlicht.a")

    # Command to run 'make' inside the Irrlicht source directory
    add_custom_command(
        OUTPUT "${IRRLICHT_LIBRARY_FILE}"
        COMMAND make NDEBUG=1
        WORKING_DIRECTORY "${IRRLICHT_ROOT}/source/Irrlicht"
        COMMENT "Compiling Irrlicht 1.8.5 from source... (This may take a minute)"
    )

    # Create a custom target that triggers the command above
    add_custom_target(IrrlichtMake DEPENDS "${IRRLICHT_LIBRARY_FILE}")

    # Create a library target that points to the file
    add_library(Irrlicht STATIC IMPORTED GLOBAL)
    add_dependencies(Irrlicht IrrlichtMake) # Force build before linking
    set_target_properties(Irrlicht PROPERTIES IMPORTED_LOCATION "${IRRLICHT_LIBRARY_FILE}")

elseif(WIN32)
    # On Windows, compiling 1.8.5 via command line is harder (needs MSBuild).
    # For now, we assume standard paths, but ideally you should commit .lib files 
    # to your repo or upgrade to a CMake-supported Irrlicht fork.
    set(IRRLICHT_LIBRARY_FILE "${IRRLICHT_ROOT}/lib/Win64-visualStudio/Irrlicht.lib")
    add_library(Irrlicht STATIC IMPORTED GLOBAL)
    set_target_properties(Irrlicht PROPERTIES IMPORTED_LOCATION "${IRRLICHT_LIBRARY_FILE}")
endif()

# ==============================================================================
# 3. Define Main Application
# ==============================================================================

# --- Define Source Files and Headers ---
# Assuming your new files are in the 'src/' directory alongside main.cpp
set(JUICEBOX_SOURCES 
    src/main.cpp
    src/Core.cpp
)

set(JUICEBOX_HEADERS 
    src/JuiceBoxEventListener.h
    src/Core.h
)

# Use the list of sources for the executable
add_executable(JuiceBox ${JUICEBOX_SOURCES})

# Inform CMake about the headers, which helps IDEs (like Visual Studio/VS Code)
target_sources(JuiceBox PRIVATE ${JUICEBOX_HEADERS})


# Link libraries
target_link_libraries(JuiceBox PRIVATE Irrlicht)

# On Linux, we also need to link X11/GL libraries to the final app
if(UNIX AND NOT APPLE)
    target_link_libraries(JuiceBox PRIVATE 
        ${X11_LIBRARIES} 
        ${X11_Xxf86vm_LIB} 
        ${OPENGL_LIBRARIES}
    )
endif()

# Add Irrlicht's include path AND the local source directory 
# for files like "JuiceBoxEventListener.h"
target_include_directories(JuiceBox PRIVATE 
    ${IRRLICHT_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)