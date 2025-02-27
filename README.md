# geomView
## Purpose
The project alows to plot structured human readeble data containig geometry primitives. <br>
The tool is usefull for visual debugging.

## Dependencies
Project UI is [imgui](https://github.com/ocornut/imgui) based.<br>
Plus you need glfw3-dev, glew-dev and opengl dev files.

## Build
### Windows
The easiest way to build *geomView* on Windows is to use *MSYS2*. It shold take you ~1.5Gb of free space.
*MSYS2* is a complete unix sub-system where you have package manager and console where you feel yourself as you are in Linux.
Yet it may not be familiar to Windows users so the build process is described in details further and as a result you supposed to get native Windows 86_x64 binary. <br>
**NOTE** geomView requires C++17 so do not try building manually with outdated Visual Studio. I suggest MSYS2 since it requires appropriate disk space and installation time.<br>
So let us get to it...
#### 1. Install MSYS2
Get MSYS2 installer from [MSYS2](https://www.msys2.org) official site<br>
After installation is complete refuse to run MSYS2 (It will run MSYS2 UCRT64 version which you do not want)

#### 2. Run MSYS MINGW64
Should look like that:
```
dan@DESKTOP-A9IF9F9 MINGW64 ~
```
**NOTE** *MINGW64* postfix in prompt<br>
From now on you are working in MSYS2 console window.

#### 3. Install gcc_w64_x86_64 from MSYS2 repository:
pacman -S mingw-w64-x86_64-gcc

#### 4. Install make from MSYS2 repository:
pacman -S make

#### 5. Install glew from MSYS2 repository:
pacman -S mingw-w64-x86_64-glew

#### 6. Get glfw3
Download 64bit GLFW3 binary from [glfw3 win64 binary](https://www.glfw.org/download.html)
and put it to your home folder (typically C:\msys64\home\%USERNAME)

#### 7. Install git from MSYS2 repository:
pacman -S git

#### 8. Install libpng from MSYS2 repository:
pacman -S mingw-w64-x86_64-libpng

#### 9. Get geomView
git clone https://github.com/dafadey/geomView.git

#### 10. Build:
Go to to geomView folder after cloning and run the following command
```
cd geomView
LIBRARY_PATH=/home/${USERNAME}/glfw-3.3.8.bin.WIN64/lib-mingw-w64 CPATH=/home/${USERNAME}/glfw-3.3.8.bin.WIN64/include make
```
**NOTE**: Check path validity for actual glfw version you downloaded.<br>
**NOTE**: Make process will clone *imgui* library. If you familiar with *imgui* library go to *Makefile*, remove *imgui* clonning commands and manually set *imgui* pathes. Also please note that *geomView* uses specific tag of *imgui* for stability reasons. *Imgui* is used as source code and not compiled as separate developer library.<br>
**NOTE**: *Makefile* uses *OS* environment variable and expects it to be "Windows_NT". Please check *env* in *MSYS2* console in case of troubles with linkage.<br>
Refer to Linux section to see the output of your build command. It should look pretty much the same in MSYS2 console.<br>
To check your build run
```
./geom_view.exe sample.txt
```
Note ./ before the commands. It is Linux style.

#### Executing geomView:
You built *geomView* 86_x64, it depends on certain dlls from Windows folder and MSYS2 folder (typically C:\msys64\mingw64\bin). You can start it right from MSYS2 console where you successfully finished build process because C:\msys64\mingw64\bin is already in PATH environment variable (check with ``echo ${PATH}`` if you want). To make geomView work from any other location you can either add C:\msys64\mingw64\bin to system environment variable PATH (This PC->Properties->Advanced system settings->Environment variables->Path->Edit use semicolon separator to add new item) or copy all needed libraries nearby your installation. Tha latter is not preffered since you have to do it every time you rebuild your geomView. With latter methiod you are manually preparing release version of geomView, by putting binary and dlls alltogether in one folder.

#### Updating geomView
Run MSYS MINGW64 console<br>
Go to geomView<br>
Run ``git pull`` to update the source code<br>
Run ``LIBRARY_PATH=/home/${USERNAME}/glfw-3.3.8.bin.WIN64/lib-mingw-w64 CPATH=/home/${USERNAME}/glfw-3.3.8.bin.WIN64/include make``
You built your up-to-date geom_view.exe!

#### Outdated OpenGL version
On GPU-less workstations/Virtual Box systems OpenGL might be limited to 1.1 version. GeomView requires GL3.3 with GLSL 330. If your CPU is powerfull enough you can download [MESA3D](https://fdossena.com/?p=mesa/index.frag) which is a software emulation of GPU. It utilizes all your cores to render the scene. Inside archive you will find opengl32.dll which you have to place right next to your geomView binary. That is it!

### Linux
Just clone and make.<br>
**NOTE**: first *make* should be executed singlethreaded and will clone imgui to the project folder.
```
dan@localhost:~/geomView$ make
cloning imgui
Cloning into 'imgui'...
remote: Enumerating objects: 45706, done.
remote: Counting objects: 100% (437/437), done.
remote: Compressing objects: 100% (193/193), done.
remote: Total 45706 (delta 317), reused 337 (delta 244), pack-reused 45269
Receiving objects: 100% (45706/45706), 85.24 MiB | 597.00 KiB/s, done.
Resolving deltas: 100% (34810/34810), done.
Note: switching to '512c54bbc062c41c74f8a8bd8ff1fd6bebd1e6d0'.

You are in 'detached HEAD' state. You can look around, make experimental
changes and commit them, and you can discard any commits you make in this
state without impacting any branches by switching back to a branch.

If you want to create a new branch to retain commits you create, you may
do so (now or later) by using -c with the switch command. Example:

  git switch -c <new-branch-name>

Or undo this operation with:

  git switch -

Turn off this advice by setting config variable advice.detachedHead to false

g++ -g -O3 -DNOIMPLOT -I./imgui -I./imgui/backends -c geo.cpp -o geo.cpp.o
g++ -g -O3 -DNOIMPLOT -I./imgui -I./imgui/backends -c draw.cpp -o draw.cpp.o
g++ -g -O3 -DNOIMPLOT -I./imgui -I./imgui/backends -c imgui_controls.cpp -o imgui_controls.cpp.o
g++ -g -O3 -DNOIMPLOT -I./imgui -I./imgui/backends -c interface.cpp -o interface.cpp.o
g++ -g -O3 -DNOIMPLOT -I./imgui -I./imgui/backends -c main.cpp -o main.cpp.o
g++ -g -O3 -DNOIMPLOT -I./imgui -I./imgui/backends -c object.cpp -o object.cpp.o
g++ -g -O3 -DNOIMPLOT -I./imgui -I./imgui/backends -c OGLitem.cpp -o OGLitem.cpp.o
g++ -g -O3 -DNOIMPLOT -I./imgui -I./imgui/backends -c saveSTL.cpp -o saveSTL.cpp.o
g++ -g -O3 -DNOIMPLOT -I./imgui -I./imgui/backends -c timer.cpp -o timer.cpp.o
g++ -g -O3 -DNOIMPLOT -I./imgui -I./imgui/backends -c tools.cpp -o tools.cpp.o
g++ -g -O3 -DNOIMPLOT -I./imgui -I./imgui/backends -c vectors.cpp -o vectors.cpp.o
g++ -g -O3 -DNOIMPLOT -I./imgui -I./imgui/backends -c imgui/imgui.cpp -o imgui/imgui.cpp.o
g++ -g -O3 -DNOIMPLOT -I./imgui -I./imgui/backends -c imgui/imgui_draw.cpp -o imgui/imgui_draw.cpp.o
g++ -g -O3 -DNOIMPLOT -I./imgui -I./imgui/backends -c imgui/imgui_tables.cpp -o imgui/imgui_tables.cpp.o
g++ -g -O3 -DNOIMPLOT -I./imgui -I./imgui/backends -c imgui/imgui_widgets.cpp -o imgui/imgui_widgets.cpp.o
g++ -g -O3 -DNOIMPLOT -I./imgui -I./imgui/backends -c imgui/backends/imgui_impl_glfw.cpp -o imgui/backends/imgui_impl_glfw.cpp.o
g++ -g -O3 -DNOIMPLOT -I./imgui -I./imgui/backends -c imgui/backends/imgui_impl_opengl3.cpp -o imgui/backends/imgui_impl_opengl3.cpp.o
g++ -g -O3 geo.cpp.o draw.cpp.o imgui_controls.cpp.o interface.cpp.o main.cpp.o object.cpp.o OGLitem.cpp.o saveSTL.cpp.o timer.cpp.o tools.cpp.o vectors.cpp.o imgui/imgui.cpp.o imgui/imgui_draw.cpp.o imgui/imgui_tables.cpp.o imgui/imgui_widgets.cpp.o imgui/backends/imgui_impl_glfw.cpp.o imgui/backends/imgui_impl_opengl3.cpp.o -o geom_view -lglfw -lGLEW -lGL
```
## Microsoft Visual Studio build
We support old versions of MSVC supporting only std11 and std14. For any MSVC version we use old standard with internal implementation of necessary std::filesystem functionality appeared in std17. To build geomViewwith MSVC you need to get and build sources for [GLEW](https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0.zip/download) and [GLFW](https://github.com/glfw/glfw/archive/refs/heads/master.zip).We suggest to use CMake-GUI to configure all three (GLEW,GLFW and geomView) projects. Everytime after first hit of Configure button in CMake-GUI please double check your settgins:

![This is an image](https://github.com/dafadey/geomView/blob/main/doc/CMake_compiler_platform_common.png)

Now let us get to static build of GLEW and GLFW. It is convenient to use static libraries to have indepentent binary and libraries.

### GLEW
Before running CMake-GUI create build and install folders. It is convenient to keep those in glew-2.1.0. Since GLEW already has build folder change some another name. Here and after we will always use build2015 for build pupposes and install2015 for installation purposes. After folders are created run CMake-GUI and set source and build folders. For GLEW it is quite counter intuitive sice CMakeLists.txt is located at glew-2.1.0/build/cmake and it is exaclty the folder treated as source by CMake. Configure step can be execute several times. Each time you set missing pathes and settings. Finally you should get the following state:

![This is an image](https://github.com/dafadey/geomView/blob/main/doc/CMake_glew.png)

After that hit Generate, Run MSVC set **Release** build type, build ALL_BUILD and then build INSTALL.

### GLFW
Do the same steps. You expected to have the following CMake configuration:

![This is an image](https://github.com/dafadey/geomView/blob/main/doc/CMake_glfw3.png)

Build and install GLFW the same way.

### geomView
Do the same steps. You expected to have the following CMake configuration:

![This is an image](https://github.com/dafadey/geomView/blob/main/doc/CMake_geomView.png)

Build and install geomView the same way. Now you have geom_view.exe and geom_view.dll in install2015/bin/, geom_view.h header in install2015/include/, static (libgeom_view.lib) and dynamic (geom_view.lib) in install2015/lib/.

### testing geomView library
Weare going to use static libgeom_view.lib in this section. In the folder testlib there is a single source test.cpp for tiny test. The test shows how to use callback function to organize interaction between console application and geom_view thread. We intentionally did not create CMakeLists.txt to show how to use geomView library from scratch since tiny projects typically do not have build systems.
First create a project nearby test.cpp:

![This is an image](https://github.com/dafadey/geomView/blob/main/doc/testlib.1_file-new-project.png)

Pathes should look as follows:

![This is an image](https://github.com/dafadey/geomView/blob/main/doc/testlib.2_file-new-project_navigate.png)

Make C++ console application

![This is an image](https://github.com/dafadey/geomView/blob/main/doc/testlib.3_file-new-project.png)

Make sure it is **empty**:

![This is an image](https://github.com/dafadey/geomView/blob/main/doc/testlib.4_app_settings.png)

Now add existing file test.cpp:

![This is an image](https://github.com/dafadey/geomView/blob/main/doc/testlib.5-add-existing-file.png)

Solution Explorer now should look as follows:

![This is an image](https://github.com/dafadey/geomView/blob/main/doc/testlib.6-file-added.png)

**Before getting to next step** set Release build type and proper architecture (**x64** in our case). Now proceed to project properties.

![This is an image](https://github.com/dafadey/geomView/blob/main/doc/testlib.7-test-Properties.png)

Set include directioes:

![This is an image](https://github.com/dafadey/geomView/blob/main/doc/testlib.8-include-dirs.png)

Set library directories:

![This is an image](https://github.com/dafadey/geomView/blob/main/doc/testlib.9-lib-dir.png)

And finally static library libgeom_view.lib and system opengl32.lib:

![This is an image](https://github.com/dafadey/geomView/blob/main/doc/testlib.10-lib.png)

After that build and run the application:

![This is an image](https://github.com/dafadey/geomView/blob/main/doc/testlib.11-result.png)

Play with control points (red stars). Each time you move control point callback is called and geometry is regerated with the code from test.cpp, dumped to disk, and then geom_view is updated with test.cpp code.

## Input format
Input format is human readable and intuitive. Look through sample.txt to get how it works<br>
Make groups of points, lines and triangles, groups can be named (A1, A2, A3 in sample.txt)<br>
For point you set coordinates and then optionally size of the point and the color
```
points : A1
(1, 1, 1)
(1, 2, 1) 3
(2, 2, 1) 3
(1.5, 1.5, 3) 4 (1, 0, 0)
```
For line set two coordinates and optionally the color.
```
triangles : A3
(3.3, 2.2, 3.3) (3.3,3.2,4.3) (5.3,3.2,4.3)
```
For trianlge set three coordinates and optionally the color.
```
lines : A2
(3.3, 2.2, 3.3) (3.3,2.2,4.3)
```

## How to use
Run with
```
./geom_view sample.txt
```
The expected result is:

![This is an image](https://github.com/dafadey/geomView/blob/main/doc/example_simple.png)

Groups A1 A2 and A3 can be hiden using check boxes.<br>

Another more complicated cases may look like this:

![This is an image](https://github.com/dafadey/geomView/blob/main/doc/example.png)

Use *open* button to add more entities. Entities can be removed or hidden with corresponding buttons.
