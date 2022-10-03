# geomView
## Purpose
The project alows to plot structured human readeble data containig geometry primitives. <br>
The tool is usefull for visual debugging.

## Dependencies
Project UI is [imgui](https://github.com/ocornut/imgui) based.<br>
Plus you need glfw3-dev, glew-dev and opengl dev files.

## Build
### Windows
Not supported but should work fine. Clone impgui yourself.
### Linux
Just clone and make.<br>
NOTE: first *make* should be executed singlethreaded and will clone imgui to the project folder.
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
![This is an image](https://github.com/dafadey/geomView/blob/main/example_simple.png)
Groups A1 A2 and A3 can be hiden using check boxes.<br>

Another more complicated cases may look like this:
![This is an image](https://github.com/dafadey/geomView/blob/main/example.png)
