CC=g++
CFLAGS=-c -Wall
LDFLAGS=-lglfw -lGL -lGLEW
SOURCES=main.cpp board.cpp board.hpp polygons.cpp polygons.hpp keybinds.cpp keybinds.hpp shaders/loadShaders.cpp shaders/loadShaders.hpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=toe

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o
