OBJ = main.o glad/src/glad.o
EXE = z80debugger
LIBS = ../libz80aw/libz80aw.a
CPPFLAGS = -Wall -Wextra -O0 -g `pkg-config --cflags glfw3` -I glad/include
LDFLAGS = `pkg-config --libs glfw3` -ldl

all: $(EXE)

$(EXE): $(OBJ) $(LIBS)
	$(CXX) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *.o **/*.o $(EXE)