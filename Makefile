TARGET_EXEC := geom_view
TARGET_LIB := libgeom_view

SOURCES := geo.cpp draw.cpp imgui_controls.cpp interface.cpp object.cpp OGLitem.cpp saveSTL.cpp timer.cpp tools.cpp vectors.cpp

SOURCES_IMGUI := imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp 

SOURCES_IMGUI_BACKENDS := imgui_impl_glfw.cpp imgui_impl_opengl3.cpp

SOURCES_ALL := $(SOURCES) $(addprefix imgui/, $(SOURCES_IMGUI)) $(addprefix imgui/backends/, $(SOURCES_IMGUI_BACKENDS))

SHADERS := sha_circle.vs sha_line.vs sha_vector.vs sha.vs sha.fs sha_line.fs sha_circle.gs sha_cp.gs sha_vector.gs

OBJS := $(addprefix obj/, $(SOURCES_ALL:%.cpp=%.o))

ifeq "$(OS)" "Windows_NT"
	libs := -lopengl32 -lglew32 -lglfw3 -lgdi32
	soext := dll
else
	libs := -lGL -lGLEW -lglfw -lrt -lm -ldl
	soext := so
endif

all : $(TARGET_EXEC) $(TARGET_LIB)

$(TARGET_EXEC): $(OBJS) obj/main.o shaderRAMfs.cpp.inl | obj
	g++ -g -fPIC obj/main.o $(OBJS) -o $@ $(libs) 

$(TARGET_LIB): $(OBJS) obj/mainlib.o shaderRAMfs.cpp.inl  | obj
	g++ -g -fPIC -shared obj/mainlib.o $(OBJS) -o $@.$(soext) $(libs) 
	ar rcs -o $@.a obj/mainlib.o $(OBJS)

#%.o: %.cpp
#	g++ -g -fPIC -O3 --std=c++17 -DNOIMPLOT -I./imgui -I./imgui/backends -c $< -o $@

include obj/Makefile.deps

shaderRAMfs.cpp.inl : $(SHADERS)
	@g++ codegen.cpp -o codegen
	./codegen $(SHADERS) > shaderRAMfs.cpp.inl

obj :
	@mkdir obj
	mkdir obj/imgui
	mkdir obj/imgui/backends

obj/Makefile.deps : shaderRAMfs.cpp.inl | obj imgui
	@bash gendeps.sh $(SOURCES_ALL) main.cpp mainlib.cpp

.PHONY: imgui
imgui:
	@if [ ! -d $@ ] ; then \
		echo cloning imgui ; \
		git clone --depth=1 -b v1.86 https://github.com/ocornut/imgui ; \
	fi

.PHONY: clean
clean:
	rm -f `ls *.o`
	rm -f $(TARGET_EXEC)
	rm -f $(TARGET_LIB).$(soext)
	rm -r obj
	rm shaderRAMfs.cpp.inl
