#include "SDL_audio.h"
#include <SDL.h>
#include <stdint.h>
#include <stdio.h>

#define MAX_CONTROLLERS 4
#define SAMPLE_RATE 48000
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
static void openGameControllers();
static void closeGameControllers();
static int initAudioDevice(int32_t samplesPerSecond, int32_t bufferSize);
static void audioCallback(void *userData, Uint8 *audioData, int length);

WindowDimension getWindowDimension(SDL_Window *window);

int main(int argc, char *argv[]) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO)) {
    const char *errorMsg = SDL_GetError();
    printf("Error initializing SDL: %s\n", errorMsg);
  }
  openGameControllers();

  int audioDeviceID = initAudioDevice(SAMPLE_RATE, 4096);

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

  closeGameControllers();
  if (audioDeviceID) {
    SDL_CloseAudioDevice(audioDeviceID);
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

static void openGameControllers() {
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
}

static void closeGameControllers() {
  for (int i = 0; i < MAX_CONTROLLERS; i++) {
    if (controllers[i]) {
      SDL_GameControllerClose(controllers[i]);
    }
  }
}

static int initAudioDevice(int32_t samplesPerSecond, int32_t bufferSize) {
  SDL_AudioSpec desiredAudioSettings = {0};
  desiredAudioSettings.freq = samplesPerSecond;
  desiredAudioSettings.format = AUDIO_S16LSB;
  desiredAudioSettings.channels = 2;
  desiredAudioSettings.samples = bufferSize;
  desiredAudioSettings.callback = &audioCallback;

  SDL_AudioSpec audioSettings;
  int deviceID =
      SDL_OpenAudioDevice(0, 0, &desiredAudioSettings, &audioSettings, 0);
  if (!deviceID) {
    const char *errorMsg = SDL_GetError();
    printf("Error opening audio device: %s\n", errorMsg);
    return 0;
  }

  printf("Initializing audio device at freq: %d Hz and channels: %d\n",
         audioSettings.freq, audioSettings.channels);

  if (audioSettings.format != AUDIO_S16LSB) {
    printf("Cannot open audio device with AUDIO_S16LSB format. Got: %d\n",
           audioSettings.format);
    SDL_CloseAudioDevice(deviceID);
  }

  SDL_PauseAudioDevice(deviceID, 0);

  return deviceID;
}

static void audioCallback(void *userData, Uint8 *stream, int length) {
  static double phase = 0; // Keeps track of position in the wave
  int16_t *buffer = (int16_t *)stream;
  int oneChanLen = length / 2;

  double const frequency = 261.63; // Middle C
  double const sampleRate = SAMPLE_RATE;

  for (int i = 0; i < oneChanLen; i += 2) {
    double sampleValue = (int16_t)(300 * sin(2.0 * M_PI * frequency * phase));

    buffer[i] = sampleValue;     // Left channel
    buffer[i + 1] = sampleValue; // Right channel

    phase += (2.0 * M_PI * frequency) / sampleRate;
    if (phase >= (2.0 * M_PI)) {
      phase -= (2.0 * M_PI);
    }
  }
}
