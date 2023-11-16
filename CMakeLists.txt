cmake_minimum_required(VERSION 3.8)
project(geom_view_Project)

include(ExternalProject)

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/imgui")
  message("cloning imgui...")
  execute_process(COMMAND git clone --depth=1 -b v1.86 https://github.com/ocornut/imgui
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()

set (CMAKE_CXX_STANDARD 17)


set(imgui_sources imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_widgets.cpp imgui/imgui_tables.cpp imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp) 

set(common_sources shaderRAMfs.cpp.inl draw.cpp geo.cpp imgui_controls.cpp interface.cpp object.cpp OGLitem.cpp saveSTL.cpp timer.cpp tools.cpp vectors.cpp ${imgui_sources})

set(sources_exec main.cpp)

set(sources_lib mainlib.cpp)

set(shaders sha_circle.vs sha_line.vs sha_vector.vs sha.vs sha.fs sha_line.fs sha_circle.gs sha_cp.gs sha_vector.gs)

set(execTarget "geom_view")
set(libTarget "geom_view_st")
set(dylibTarget "geom_view_dyn")

if(WIN32)
  set(opts /DNOIMPLOT)
else()
  set(opts -fPIC -g -O3 -DNOIMPLOT)
endif()

message("opts ${opts}")
add_compile_options(${opts})

add_executable(${execTarget} ${sources_exec})
add_library(${libTarget} STATIC ${sources_lib})
add_library(${dylibTarget} SHARED ${sources_lib})

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

set(OpenGL_GL_PREFERENCE LEGACY)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
message("GLEW: ${GLEW_INCLUDE_DIR}")
message("glfw3: ${glfw3_DIR}")

#unfortunatelly lgfw3 does not set glfw3_INCLUDE_DIR so we have to find
if(NOT DEFINED ${glfw3_INCLUDE_DIR})
  if(EXISTS "${glfw3_DIR}/../../../include/GLFW")
    set(glfw3_INCLUDE_DIR "${glfw3_DIR}/../../../include")
  endif()
  if(EXISTS "${glfw3_DIR}/../../include/GLFW")
    set(glfw3_INCLUDE_DIR "${glfw3_DIR}/../../include")
  endif()
endif()

message("glfw3_INCLUDE_DIR: ${glfw3_INCLUDE_DIR}")

include_directories(imgui imgui/backends ${GLEW_INCLUDE_DIR} ${glfw3_INCLUDE_DIR})

target_link_libraries(${execTarget} PUBLIC commonArch ${OPENGL_LIBRARIES} glfw ${GLEW_LIBRARIES})
target_link_libraries(${dylibTarget} PUBLIC commonArch ${OPENGL_LIBRARIES} glfw ${GLEW_LIBRARIES})

message("OpenGL libraries:=${OPENGL_LIBRARIES}")

if(WIN32)
    target_link_libraries(${execTarget} PUBLIC gdi32.lib)
    target_link_libraries(${dylibTarget} PUBLIC gdi32.lib)
else()
    target_link_libraries(${execTarget} PUBLIC -lrt -lm -ldl)
    target_link_libraries(${dylibTarget} PUBLIC -lrt -lm -ldl)
endif()
