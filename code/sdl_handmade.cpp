#include "SDL.h"

SDL_Window *Window;

bool HandleEvent(SDL_Event *Event);

int main(int argc, char *argv[]) {
  if (SDL_Init(SDL_INIT_VIDEO)) {
    // TODO: make something in here
  }

  Window =
      SDL_CreateWindow("Handmade Hero", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_RESIZABLE);

  for (;;) {
    SDL_Event Event;
    SDL_WaitEvent(&Event);
    if (HandleEvent(&Event)) {
      break;
    }
  }

  return 0;
}

bool HandleEvent(SDL_Event *Event) {
  bool ShouldQuit = false;

  switch (Event->type) {
  case SDL_QUIT: {
    printf("SDL_QUIT\n");
    ShouldQuit = true;
  } break;
  case SDL_WINDOWEVENT: {
    switch (Event->window.event) {
    case SDL_WINDOWEVENT_RESIZED: {
      printf("SDL_WINDOWEVENT_RESIZED (%d x %d)\n", Event->window.data1,
             Event->window.data2);
    } break;
    }
  }
  }
  return (ShouldQuit);
}
