cmake_minimum_required(VERSION 3.8)
project(geom_view_project)

include(ExternalProject)

set(GIT_EXECUTABLE "git" CACHE STRING "git executable with full path")

#find_package(Git)
#if(Git_FOUND)
#  message("git found: ${GIT_EXECUTABLE}")
#else()
#  unset(GIT_EXECUTABLE CACHE)
#  set(GIT_EXECUTABLE "NOTFOUND" CACHE STRING "git executable with full path")
#  message("git was not found")
#endif()

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/imgui")
  message("cloning imgui...")
  execute_process(COMMAND ${GIT_EXECUTABLE} clone --depth=1 -b v1.86 https://github.com/ocornut/imgui
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} RESULT_VARIABLE ret)
  if(NOT (ret EQUAL "0"))
    message( FATAL_ERROR "git does not work output is {${ret}}")
    unset(GIT_EXECUTABLE CACHE)
    set(GIT_EXECUTABLE "NOTFOUND" CACHE STRING "git executable with full path")
  endif()
endif()

if(MSVC)
  add_definitions(-DUNICODE -D_UNICODE -DNOSTDFILESYSTEM 
  -DGLEWINIT_INTERNAL)
  set(CMAKE_CXX_STANDARD 11)
else()
  set(CMAKE_CXX_STANDARD 17)
endif()

set(imgui_sources imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_widgets.cpp imgui/imgui_tables.cpp imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp) 

set(common_sources custom_controls.cpp buttons.png.inl shaderRAMfs.cpp.inl glass_buttons.cpp fb2way.cpp shaders.cpp draw.cpp geo.cpp imgui_controls.cpp interface.cpp object.cpp OGLitem.cpp saveSTL.cpp timer.cpp tools.cpp vectors.cpp ${imgui_sources})

set(sources_exec main.cpp)

set(sources_lib mainlib.cpp)

set(shaders sha_circle.vs sha_line.vs sha_vector.vs sha.vs sha.fs sha_line.fs sha_circle.gs sha_cp.gs sha_vector.gs)

set(execTarget "geom_view")
set(libNAME "geom_view")
set(libTarget "geom_view_st")
set(dylibTarget "geom_view_dyn")

if(WIN32)
  set(opts /DNOIMPLOT)
else()
  set(opts -fPIC -g -O3 -DNOIMPLOT)
endif()

message("opts ${opts}")
add_compile_options(${opts})

set(PUB_HEADERS geom_view.h custom_controls.h)

add_executable(${execTarget} ${sources_exec})
add_library(${libTarget} STATIC ${PUB_HEADERS} ${sources_lib})

add_library(${dylibTarget} SHARED ${PUB_HEADERS} ${sources_lib})

set(MSVC_LIB_DEFINITIONS "DYNAMIC_BUILD")
if(MSVC)
	set(MSVC_LIB_DEFINITIONS "MSVC_DYNAMIC_BUILD")
endif()

set_target_properties (${dylibTarget} PROPERTIES COMPILE_DEFINITIONS 
					   ${MSVC_LIB_DEFINITIONS} OUTPUT_NAME "${libNAME}" PREFIX "")

set_target_properties(${libTarget} PROPERTIES PUBLIC_HEADER "${PUB_HEADERS}")
set_target_properties(${dylibTarget} PROPERTIES PUBLIC_HEADER "${PUB_HEADERS}")

add_library(commonArch STATIC ${common_sources})

add_dependencies(${execTarget} commonArch)
add_dependencies(${dylibTarget} commonArch)

add_executable(charcodegen codegen.cpp)

add_custom_command(OUTPUT shaderRAMfs.cpp.inl
                   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                   COMMAND charcodegen ${shaders} > shaderRAMfs.cpp.inl
                   DEPENDS charcodegen ${shaders}
                   VERBATIM)

add_executable(charcodegenbin codegenbin.cpp)

add_custom_command(OUTPUT buttons.png.inl
                   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                   COMMAND charcodegenbin buttons.png > buttons.png.inl
                   DEPENDS charcodegenbin buttons.png
                   VERBATIM)

set(OpenGL_GL_PREFERENCE LEGACY)

find_package(OpenGL REQUIRED)
message("OPENGL_LIBRARIES=${OPENGL_LIBRARIES}")
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
#find_package(PNG REQUIRED)

message("libpng: ${PNG_INCLUDE_DIR}")
message("libpng lib: ${PNG_LIBRARY}")
message("libpng lib: ${ZLIB_LIBRARY}")

message("GLEW: ${GLEW_INCLUDE_DIR}")
message("glfw3: ${glfw3_DIR}")

message("glfw3 lib: ${glfw3_LIBRARY}")

#unfortunatelly glfw3 does not set glfw3_INCLUDE_DIR so we have to find
if(NOT DEFINED ${glfw3_INCLUDE_DIR})
  if(EXISTS "${glfw3_DIR}/../../../include/GLFW")
    set(glfw3_INCLUDE_DIR "${glfw3_DIR}/../../../include")
    set(glfw3_LIBRARY "${glfw3_DIR}/../../../lib/glfw3.lib")
  endif()
  if(EXISTS "${glfw3_DIR}/../../include/GLFW")
    set(glfw3_INCLUDE_DIR "${glfw3_DIR}/../../include")
    set(glfw3_LIBRARY "${glfw3_DIR}/../../lib/glfw3.lib")
  endif()
endif()

message("glfw3_INCLUDE_DIR: ${glfw3_INCLUDE_DIR}")

include_directories(imgui imgui/backends ${GLEW_INCLUDE_DIR} ${PNG_INCLUDE_DIR} ${glfw3_INCLUDE_DIR})

if (MSVC)
	set(GLEW_LIBS "${GLEW_DIR}/../../libglew32.lib")
else()
	set(GLEW_LIBS "${GLEW_LIBRARIES}")
endif()

message("GLEW_LIBS=${GLEW_LIBS}")

message("GLEW_STATIC_LIBRARIES=${GLEW_STATIC_LIBS}")
target_link_libraries(${execTarget} PUBLIC commonArch ${OPENGL_LIBRARIES} glfw ${GLEW_LIBS} ${PNG_LIBRARY} ${ZLIB_LIBRARY})
target_link_libraries(${dylibTarget} LINK_PUBLIC commonArch ${OPENGL_LIBRARIES} glfw ${GLEW_LIBS} ${PNG_LIBRARY} ${ZLIB_LIBRARY})

message("CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")

if(MSVC)
    add_custom_command(
        OUTPUT lib${libNAME}.lib
        COMMAND lib.exe /OUT:${CMAKE_BINARY_DIR}/$<CONFIG>/lib${libNAME}.lib $<TARGET_FILE:commonArch> $<TARGET_FILE:${libTarget}> ${GLEW_LIBS} ${glfw3_LIBRARY} ${PNG_LIBRARY} ${ZLIB_LIBRARY}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        DEPENDS commonArch ${PUB_HEADERS} ${sources_lib}
        COMMENT "linking static library lib${libNAME}.lib"
        )

    add_custom_target(combinedLibGenerator ALL
        DEPENDS lib${libNAME}.lib
    )
endif()

add_library(combinedLib STATIC IMPORTED)
set_property(TARGET combinedLib PROPERTY IMPORTED_LOCATION lib${libNAME}.lib)
add_dependencies(combinedLib combinedLibGenerator)

message("OpenGL libraries:=${OPENGL_LIBRARIES}")

if(WIN32)
    target_link_libraries(${execTarget} PUBLIC gdi32.lib)
    target_link_libraries(${dylibTarget} LINK_PUBLIC gdi32.lib)
else()
    target_link_libraries(${execTarget} PUBLIC -lrt -lm -ldl -lpng)
    target_link_libraries(${dylibTarget} LINK_PUBLIC -lrt -lm -ldl -lpng)
endif()

list(APPEND install_targets ${execTarget})
list(APPEND install_targets ${dylibTarget})

install(TARGETS ${install_targets}
          RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
          LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
          ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
          PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_PREFIX}/include
)

install(FILES ${CMAKE_BINARY_DIR}/$<CONFIG>/lib${libNAME}.lib
        DESTINATION lib
)
