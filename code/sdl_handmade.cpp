#include <SDL.h>
#include <stdint.h>
#include <stdlib.h>

struct BackBuffer {
  SDL_Texture *texture;
  void *memory;
  int width;
  int height;
  int pitch;
  int bytesPerPixel;
};

struct WindowDimension {
  int width;
  int height;
};

static BackBuffer globalBackBuffer;

int xOffset = 128;
int yOffset = 0;

bool handleEvent(SDL_Event *event);
static void updateWindow(SDL_Window *window, SDL_Renderer *renderer,
                         BackBuffer buffer);
static void renderWeirdGradient(BackBuffer buffer, int xOffset, int yOffset);
static void resizeTexture(BackBuffer *buffer, SDL_Renderer *renderer, int width,
                          int height);

WindowDimension getWindowDimension(SDL_Window *window);

int main(int argc, char *argv[]) {
  if (SDL_Init(SDL_INIT_VIDEO)) {
    // TODO: make something in here
  }

  SDL_Window *window =
      SDL_CreateWindow("Handmade Hero", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_RESIZABLE);
  if (!window) {
    return 1;
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

  if (!renderer) {
    return 1;
  }

  WindowDimension dimension = getWindowDimension(window);
  resizeTexture(&globalBackBuffer, renderer, dimension.width, dimension.height);

  bool running = true;
  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (handleEvent(&event)) {
        running = false;
      }
    }
    renderWeirdGradient(globalBackBuffer, xOffset, yOffset);
    updateWindow(window, renderer, globalBackBuffer);

    ++xOffset;
    yOffset += 1;
  }

  SDL_Quit();
  return 0;
}

WindowDimension getWindowDimension(SDL_Window *window) {
  WindowDimension dimension;

  SDL_GetWindowSize(window, &dimension.width, &dimension.height);

  return dimension;
}

bool handleEvent(SDL_Event *event) {
  bool shouldQuit = false;

  switch (event->type) {
  case SDL_QUIT: {
    printf("SDL_QUIT\n");
    shouldQuit = true;
  } break;
  case SDL_WINDOWEVENT: {
    switch (event->window.event) {
    case SDL_WINDOWEVENT_SIZE_CHANGED: {
      printf("SDL_WINDOWEVENT_RESIZED (%d x %d)\n", event->window.data1,
             event->window.data2);
      SDL_Window *window = SDL_GetWindowFromID(event->window.windowID);
      SDL_Renderer *renderer = SDL_GetRenderer(window);
      resizeTexture(&globalBackBuffer, renderer, event->window.data1,
                    event->window.data2);
      renderWeirdGradient(globalBackBuffer, xOffset, yOffset);
      updateWindow(window, renderer, globalBackBuffer);
    } break;
    case SDL_WINDOWEVENT_EXPOSED: {
      SDL_Window *window = SDL_GetWindowFromID(event->window.windowID);
      SDL_Renderer *renderer = SDL_GetRenderer(window);
      updateWindow(window, renderer, globalBackBuffer);
      printf("Exposing\n");
    } break;
    }
  }
  }
  return shouldQuit;
}

static void resizeTexture(BackBuffer *buffer, SDL_Renderer *renderer, int width,
                          int height) {
  if (buffer->memory) {
    free(buffer->memory);
  }

  if (buffer->texture) {
    SDL_DestroyTexture(buffer->texture);
  }

  buffer->texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_STREAMING, width, height);
  buffer->width = width;
  buffer->height = height;
  buffer->bytesPerPixel = 4;
  buffer->pitch = width * buffer->bytesPerPixel;
  buffer->memory =
      malloc(buffer->width * buffer->height * buffer->bytesPerPixel);
}

static void updateWindow(SDL_Window *window, SDL_Renderer *renderer,
                         BackBuffer buffer) {
  if (SDL_UpdateTexture(buffer.texture, 0, buffer.memory, buffer.pitch)) {
    // TODO: Handle error here
  }

  SDL_RenderCopy(renderer, buffer.texture, 0, 0);
  SDL_RenderPresent(renderer);
}

static void renderWeirdGradient(BackBuffer buffer, int xOffset, int yOffset) {
  // Pitch - number of bytes that represent on row of pixels in bitmap or
  // texture
  uint8_t *row = (uint8_t *)buffer.memory;

  for (int y = 0; y < buffer.height; y++) {
    uint32_t *pixel = (uint32_t *)row;
    for (int x = 0; x < buffer.width; x++) {
      uint8_t blue = x + xOffset;
      uint8_t green = y + yOffset;
      *pixel++ = (green << 8 | blue);
    }
    row += buffer.pitch;
  }
}
