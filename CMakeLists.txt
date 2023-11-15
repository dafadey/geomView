cmake_minimum_required(VERSION 3.5)
project(geom_view_Project)

include(ExternalProject)

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/imgui")
  message("cloning imgui...")
  execute_process(COMMAND git clone --depth=1 -b v1.86 https://github.com/ocornut/imgui
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()

set(imgui_sources imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_widgets.cpp imgui/imgui_tables.cpp imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp) 

set(common_sources shaderRAMfs.cpp.inl draw.cpp geo.cpp imgui_controls.cpp interface.cpp object.cpp OGLitem.cpp saveSTL.cpp timer.cpp tools.cpp vectors.cpp ${imgui_sources})

set(sources_exec main.cpp)

set(sources_lib mainlib.cpp)

set(shaders sha_circle.vs sha_line.vs sha_vector.vs sha.vs sha.fs sha_line.fs sha_circle.gs sha_cp.gs sha_vector.gs)

set(execTarget "geom_view")
set(libTarget "geom_view_st")
set(dylibTarget "geom_view_dyn")

add_executable(${execTarget} ${sources_exec})
add_library(${libTarget} STATIC ${sources_lib})
add_library(${dylibTarget} SHARED ${sources_lib})

set(opts -fPIC --std=c++17 -g -O3 -DNOIMPLOT)

add_library(commonArch STATIC ${common_sources})

add_dependencies(${execTarget} commonArch)
add_dependencies(${libTarget} commonArch)
add_dependencies(${dylibTarget} commonArch)

add_executable(codegen codegen.cpp)

add_custom_command(OUTPUT shaderRAMfs.cpp.inl
                   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                   COMMAND codegen ${shaders} > shaderRAMfs.cpp.inl
                   DEPENDS codegen ${shaders}
                   VERBATIM)

target_compile_options(commonArch PUBLIC ${opts})
target_compile_options(${execTarget} PUBLIC ${opts})
target_compile_options(${libTarget} PUBLIC ${opts})
target_compile_options(${dylibTarget} PUBLIC ${opts})

target_include_directories(commonArch PUBLIC imgui imgui/backends)
target_include_directories(${execTarget} PUBLIC imgui imgui/backends)
target_include_directories(${libTarget} PUBLIC imgui imgui/backends)
target_include_directories(${dylibTarget} PUBLIC imgui imgui/backends)

set(OpenGL_GL_PREFERENCE LEGACY)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

target_link_libraries(${execTarget} PUBLIC commonArch OpenGL glfw GLEW -lrt -lm -ldl)
target_link_libraries(${dylibTarget} PUBLIC commonArch OpenGL glfw GLEW -lrt -lm -ldl)

if(WIN32)
    target_link_libraries(${execTarget} PUBLIC -lgdi32)
    target_link_libraries(${dylibTarget} PUBLIC -lgdi32)
endif(WIN32)

if(UNIX)
    target_link_libraries(${execTarget} PUBLIC -lrt -lm -ldl)
    target_link_libraries(${dylibTarget} PUBLIC -lrt -lm -ldl)
endif(UNIX)
