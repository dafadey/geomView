TARGET_EXEC := geom_view
TARGET_LIB := libgeom_view

SOURCES := glass_buttons.cpp fb2way.cpp shaders.cpp geo.cpp draw.cpp imgui_controls.cpp interface.cpp object.cpp OGLitem.cpp saveSTL.cpp timer.cpp tools.cpp vectors.cpp

SOURCES_IMGUI := imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp 

SOURCES_IMGUI_BACKENDS := imgui_impl_glfw.cpp imgui_impl_opengl3.cpp

SOURCES_ALL := $(SOURCES) $(addprefix imgui/, $(SOURCES_IMGUI)) $(addprefix imgui/backends/, $(SOURCES_IMGUI_BACKENDS))

SHADERS := sha_circle.vs sha_line.vs sha_vector.vs sha.vs sha.fs sha_line.fs sha_circle.gs sha_cp.gs sha_vector.gs

OBJS := $(addprefix obj/, $(SOURCES_ALL:%.cpp=%.o))

ifeq "$(OS)" "Windows_NT"
	libs := -v -lopengl32 -lglfw3 -lglew32 -lgdi32 -lole32 -static-libgcc -static-libstdc++ -Wl,-Bstatic -lpthread -lpng -lz
	soext := dll
else
	libs := -lGL -lGLEW -lglfw -lrt -lm -ldl -lpng
	soext := so
endif

all : $(TARGET_EXEC) $(TARGET_LIB)

$(TARGET_EXEC): $(OBJS) obj/main.o obj
	g++ -g -fPIC obj/main.o $(OBJS) -o $@ $(libs) 

$(TARGET_LIB): $(OBJS) obj/mainlib.o obj
	g++ -g -fPIC -shared obj/mainlib.o $(OBJS) -o $@.$(soext) $(libs) 
	echo "create $@.a\naddmod obj/mainlib.o $OBJS\nsave\nexit\n" | ar -M

$(TARGET_LIB): $(OBJS) obj/mainlib.o obj arscript
	g++ -g -fPIC -shared obj/mainlib.o $(OBJS) -o $@.$(soext) $(libs) 
	if [ -d libglfw_objs ]; then ar rcs -o $@.a libglfw_objs/*.obj obj/mainlib.o $(OBJS); \
	else ar rcs -o $@.a obj/mainlib.o $(OBJS); \
	fi

arscript :
	if [ -e ${LIBRARY_PATH}/libglfw3.a ]; then mkdir -p libglfw_objs; \
	cp ${LIBRARY_PATH}/libglfw3.a libglfw_objs/; \
	cd libglfw_objs; \
	ar -x libglfw3.a; \
	fi
	
#%.o: %.cpp
#	g++ -g -fPIC -O3 --std=c++17 -DNOIMPLOT -I./imgui -I./imgui/backends -c $< -o $@

include obj/Makefile.deps

shaderRAMfs.cpp.inl : $(SHADERS)
	echo making $@ from $<
	@g++ codegen.cpp -o codegen
	./codegen $(SHADERS) > $@

buttons.png.inl : buttons.png
	echo making $@
	@g++ codegenbin.cpp -o codegenbin
	./codegenbin $< > $@

obj :
	@mkdir -p obj
	mkdir -p obj/imgui
	mkdir -p obj/imgui/backends

obj/Makefile.deps : shaderRAMfs.cpp.inl buttons.png.inl | obj imgui
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
