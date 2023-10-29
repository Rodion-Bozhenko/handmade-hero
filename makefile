CC = clang++
SDL_FLAGS = $(shell sdl2-config --cflags --libs)
CFLAGS = -g
OUTPUT = build/handmadehero
SOURCES = code/sdl_handmade.cpp code/handmade.cpp

all: $(OUTPUT)

$(OUTPUT): $(SOURCES)
	mkdir -p build
	$(CC) $(SOURCES) -o $(OUTPUT) $(SDL_FLAGS) $(CFLAGS)

run: $(OUTPUT)
	./$(OUTPUT)
	
debug: $(OUTPUT)
	lldb ./$(OUTPUT)

clean:
	rm -rf build

.PHONY: all run clean
