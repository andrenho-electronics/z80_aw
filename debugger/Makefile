OBJ = main.o window.o ui.o myui.o options.o config.o
OBJ_CONTRIB = glad/src/glad.o imgui/imgui.o imgui/imgui_demo.o imgui/imgui_draw.o imgui/imgui_impl_glfw.o imgui/imgui_impl_opengl3.o imgui/imgui_tables.o imgui/imgui_widgets.o
EXE = z80debugger
LIBS = ../libz80aw/libz80aw.a
CPPFLAGS = -Wall -Wextra -O0 -g `pkg-config --cflags glfw3` -Iglad/include -I../libz80aw
CXXFLAGS = -std=c++17
LDFLAGS = `pkg-config --libs glfw3` -ldl

all: $(EXE)

$(LIBS):
	$(MAKE) -C ../libz80aw

$(EXE): $(OBJ) $(OBJ_CONTRIB) $(LIBS)
	$(CXX) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *.o **/*.o $(EXE)

# vim: set ts=8 sts=8 sw=8 noexpandtab:
