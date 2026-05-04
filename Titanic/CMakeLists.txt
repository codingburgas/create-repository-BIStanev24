cmake_minimum_required(VERSION 3.10)
project(Titanic)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Define Paths
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui")
set(GLFW_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/glfw")

# 2. Define all Source Files
set(ALL_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    src/data.cpp
    src/logic.cpp
    src/main.cpp
    src/ui.cpp
)

# 3. Setup Include Directories
include_directories(${IMGUI_DIR})
include_directories(${IMGUI_DIR}/backends)
include_directories(${GLFW_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 4. Create the Executable
add_executable(Titanic ${ALL_SOURCES})

# 5. Link Libraries (Points to the folder you just added)
if(WIN32)
    # Link to the actual .lib file inside your new lib-vc2022 folder
    target_link_libraries(Titanic PRIVATE 
        "${GLFW_DIR}/lib-vc2022/glfw3.lib"
        opengl32
    )
endif()