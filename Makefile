TARGET_EXEC := geom_view

SOURCES := geo.cpp draw.cpp imgui_controls.cpp interface.cpp main.cpp object.cpp OGLitem.cpp saveSTL.cpp timer.cpp tools.cpp vectors.cpp

SOURCES_IMGUI := imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp

OBJS := $(SOURCES:%=%.o) $(SOURCES_IMGUI:%=%.o)

$(TARGET_EXEC): imgui $(OBJS)
	g++ -g -O3 $(OBJS) -o $@ -lglfw -lGLEW -lGL

%.cpp.o: %.cpp
	g++ -g -O3 -DNOIMPLOT -I./imgui -I./imgui/backends -c $< -o $@

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
