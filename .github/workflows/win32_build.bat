set WD=%cd%

echo current working directory is %WD%

set glew=glew-cmake-glew-cmake-2.1.0

REM GLEW
call :buildURL https://github.com/Perlmint/glew-cmake/archive/refs/tags/glew-cmake-2.1.0.tar.gz glew-2.1.0.tar.gz glew-cmake-glew-cmake-2.1.0 "-Dglew-cmake_BUILD_SHARED=OFF"
copy glew-cmake-glew-cmake-2.1.0\install_MSVC\lib\glew.lib glew-cmake-glew-cmake-2.1.0\install_MSVC\lib\libglew32.lib
if not exist %glew%\install_MSVC\lib\cmake mkdir %glew%\install_MSVC\lib\cmake
if not exist %glew%\install_MSVC\lib\cmake\glew mkdir %glew%\install_MSVC\lib\cmake\glew
echo add_library(GLEW::GLEW SHARED IMPORTED) > %glew%\install_MSVC\lib\cmake\glew\glewConfig.cmake
echo set_target_properties(GLEW::GLEW PROPERTIES >> %glew%\install_MSVC\lib\cmake\glew\glewConfig.cmake
echo   INTERFACE_INCLUDE_DIRECTORIES "%glew%/install_MSVC/include" >> %glew%\install_MSVC\lib\cmake\glew\glewConfig.cmake
echo   INTERFACE_LINK_LIBRARIES "opengl32;glu32") >> %glew%\install_MSVC\lib\cmake\glew\glewConfig.cmake

REM GLFW
call :buildURL https://github.com/glfw/glfw/archive/refs/tags/3.4.tar.gz glfw-3.4.tar.gz glfw-3.4 "-DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF"
REM ZLIB
call :buildURL http://zlib.net/zlib-1.3.1.tar.gz zlib-1.3.1.tar.gz zlib-1.3.1 "-DZLIB_BUILD_EXAMPLES=OFF"
REM LIBPNG
call :buildURL http://prdownloads.sourceforge.net/libpng/libpng-1.6.49.tar.gz libpng-1.6.49.tar.gz libpng-1.6.49 "-DZLIB_ROOT=%WD%\zlib-1.3.1\install_MSVC -DPNG_SHARED=OFF -DPNG_TESTS=OFF -DPNG_TOOLS=OFF"

REM GEOMVIEW
call :build "-DGLEW_DIR=%WD%\glew-cmake-glew-cmake-2.1.0\install_MSVC\lib\cmake\glew -Dglfw3_DIR=%WD%\glfw-3.4\install_MSVC\lib\cmake\glfw3 -DPNG_INCLUDE_DIR=%WD%\libpng-1.6.49\install_MSVC\include -DPNG_LIBRARY=%WD%\libpng-1.6.49\install_MSVC\lib\libpng16_static.lib -DZLIB_LIBRARY=%WD%\zlib-1.3.1\install_MSVC\lib\zlibstatic.lib"

echo.&goto:eof

:buildURL
if not exist %~2 curl -o %~2 -L -O %~1
if not exist %~3 tar xvf %~2
cd %~3 || goto:eof
if not exist build_MSVC mkdir build_MSVC
if not exist install_MSVC mkdir install_MSVC
cd build_MSVC
cmake ../ -DCMAKE_INSTALL_PREFIX=../install_MSVC %~4
cmake --build . --config Release --target INSTALL
cd ../../
goto:eof

:build
if not exist build_MSVC mkdir build_MSVC
if not exist install_MSVC mkdir install_MSVC
cd build_MSVC
cmake ../ -DCMAKE_INSTALL_PREFIX=../install_MSVC %~1
cmake --build . --config Release --target INSTALL
cd ../
goto:eof
