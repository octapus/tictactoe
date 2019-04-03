CC=g++
CFLAGS=-c -Wall
LDFLAGS=-lglfw -lGL -lGLEW -pthread
SOURCES=main.cpp board.cpp polygons.cpp keybinds.cpp shaders/loadShaders.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=toe

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o
