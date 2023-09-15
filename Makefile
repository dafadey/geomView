TARGET_EXEC := geom_view
TARGET_LIB := libgeom_view

SOURCES := geo.cpp draw.cpp imgui_controls.cpp interface.cpp object.cpp OGLitem.cpp saveSTL.cpp timer.cpp tools.cpp vectors.cpp

SOURCES_IMGUI := imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp

OBJS := $(SOURCES:%=%.o) $(SOURCES_IMGUI:%=%.o)

ifeq "$(OS)" "Windows_NT"
	libs := -lopengl32 -lglew32 -lglfw3 -lgdi32
else
	libs := -lGL -lGLEW -lglfw -lrt -lm -ldl
endif

all : $(TARGET_EXEC) $(TARGET_LIB)

$(TARGET_EXEC): imgui $(OBJS) main.cpp.o
	g++ -g -fPIC main.cpp.o $(OBJS) -o $@ $(libs) 

$(TARGET_LIB): imgui $(OBJS) mainlib.cpp.o
	g++ -g -fPIC -shared mainlib.cpp.o $(OBJS) -o $@.so $(libs) 

%.cpp.o: %.cpp
	g++ -g -fPIC -O3 --std=c++17 -DNOIMPLOT -I./imgui -I./imgui/backends -c $< -o $@

.PHONY: imgui
imgui:
	@if [ ! -d $@ ] ; then \
		echo cloning imgui ; \
		git clone -b v1.86 https://github.com/ocornut/imgui ; \
	fi

.PHONY: clean
clean:
	rm -f `ls *.o`
	rm -f `ls imgui/*.o`
	rm -f `ls imgui/backends/*.o`
	rm -f $(TARGET_EXEC)
	rm -f $(TARGET_LIB).so
