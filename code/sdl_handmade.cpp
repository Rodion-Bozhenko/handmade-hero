#include <SDL.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_CONTROLLERS 4
SDL_GameController *controllers[MAX_CONTROLLERS];

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

int xOffset = 0;
int yOffset = 0;

bool handleEvent(SDL_Event *event);
static void updateWindow(SDL_Window *window, SDL_Renderer *renderer,
                         BackBuffer buffer);
static void renderWeirdGradient(BackBuffer buffer, int xOffset, int yOffset);
static void resizeTexture(BackBuffer *buffer, SDL_Renderer *renderer, int width,
                          int height);

WindowDimension getWindowDimension(SDL_Window *window);

int main(int argc, char *argv[]) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER)) {
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

    int maxJoysticks = SDL_NumJoysticks();
    for (int i = 0; i < maxJoysticks; i++) {
      if (!SDL_IsGameController(i)) {
        continue;
      }
      if (i >= MAX_CONTROLLERS) {
        break;
      }
      controllers[i] = SDL_GameControllerOpen(i);
    }

    for (int i = 0; i < MAX_CONTROLLERS; i++) {
      if (SDL_GameControllerGetAttached(controllers[i])) {
        bool up = SDL_GameControllerGetButton(controllers[i],
                                              SDL_CONTROLLER_BUTTON_DPAD_UP);
        bool down = SDL_GameControllerGetButton(
            controllers[i], SDL_CONTROLLER_BUTTON_DPAD_DOWN);
        bool left = SDL_GameControllerGetButton(
            controllers[i], SDL_CONTROLLER_BUTTON_DPAD_LEFT);
        bool right = SDL_GameControllerGetButton(
            controllers[i], SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
        bool start = SDL_GameControllerGetButton(controllers[i],
                                                 SDL_CONTROLLER_BUTTON_START);
        bool back = SDL_GameControllerGetButton(controllers[i],
                                                SDL_CONTROLLER_BUTTON_BACK);
        bool leftShoulder = SDL_GameControllerGetButton(
            controllers[i], SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
        bool rightShoulder = SDL_GameControllerGetButton(
            controllers[i], SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
        bool aButton = SDL_GameControllerGetButton(controllers[i],
                                                   SDL_CONTROLLER_BUTTON_A);
        bool bButton = SDL_GameControllerGetButton(controllers[i],
                                                   SDL_CONTROLLER_BUTTON_B);
        bool xButton = SDL_GameControllerGetButton(controllers[i],
                                                   SDL_CONTROLLER_BUTTON_X);
        bool yButton = SDL_GameControllerGetButton(controllers[i],
                                                   SDL_CONTROLLER_BUTTON_Y);

        int16_t stickX = SDL_GameControllerGetAxis(controllers[i],
                                                   SDL_CONTROLLER_AXIS_LEFTX);
        int16_t stickY = SDL_GameControllerGetAxis(controllers[i],
                                                   SDL_CONTROLLER_AXIS_LEFTY);

        if (aButton) {
          yOffset += 2;
        }
      }
    }
  }

  for (int i = 0; i < MAX_CONTROLLERS; i++) {
    if (controllers[i]) {
      SDL_GameControllerClose(controllers[i]);
    }
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
  case SDL_KEYDOWN:
  case SDL_KEYUP: {
    SDL_Keycode key = event->key.keysym.sym;
    bool wasDown = false;
    if (event->key.state == SDL_RELEASED) {
      wasDown = true;
    }
    if (event->key.repeat != 0) {
      wasDown = true;
    }
    if (!wasDown) {
      switch (key) {
      case SDLK_w: {
        printf("w\n");
        yOffset += 200;
      } break;
      case SDLK_s: {
        printf("s\n");
        yOffset -= 200;
      } break;
      case SDLK_d: {
        printf("d\n");
        xOffset += 200;
      } break;
      case SDLK_a: {
        printf("a\n");
        xOffset -= 200;
      } break;
      }
    }
  } break;
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
