#include <SDL.h>
#include <SDL_pixels.h>
#include <stdlib.h>

bool HandleEvent(SDL_Event *Event);
static void SDLUpdateWindow(SDL_Window *Window);

int BitmapWidth;
int BytesPerPixel = 4;

int main(int argc, char *argv[]) {
  if (SDL_Init(SDL_INIT_VIDEO)) {
    // TODO: make something in here
  }

  SDL_Window *Window =
      SDL_CreateWindow("Handmade Hero", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_RESIZABLE);
  if (!Window) {
    return 1;
  }

  SDL_Renderer *Renderer = SDL_CreateRenderer(Window, -1, 0);

  if (!Renderer) {
    return 1;
  }

  for (;;) {
    SDL_Event Event;
    SDL_WaitEvent(&Event);
    if (HandleEvent(&Event)) {
      break;
    }
  }

  SDL_Quit();
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
    case SDL_WINDOWEVENT_SIZE_CHANGED: {
      printf("SDL_WINDOWEVENT_RESIZED (%d x %d)\n", Event->window.data1,
             Event->window.data2);

    } break;
    case SDL_WINDOWEVENT_EXPOSED: {
      SDL_Window *Window = SDL_GetWindowFromID(Event->window.windowID);
      SDLUpdateWindow(Window);
    } break;
    }
  }
  }
  return ShouldQuit;
}

static void SDLUpdateWindow(SDL_Window *Window) {
  SDL_Renderer *Renderer = SDL_GetRenderer(Window);

  int Width, Height;
  SDL_GetWindowSize(Window, &Width, &Height);
  BitmapWidth = Width;

  SDL_Texture *Texture =
      SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_STREAMING, Width, Height);

  void *BitmapMemory = malloc(Width * Height * BytesPerPixel);

  // Make it white
  int *p = (int *)BitmapMemory;
  for (int i = 0; i < Width * Height; i++) {
    *p++ = 0xffffffff;
  }

  int pitch = BitmapWidth * BytesPerPixel;
  if (SDL_UpdateTexture(Texture, 0, BitmapMemory, pitch)) {
    // TODO: Handle error here
  }
  SDL_RenderCopy(Renderer, Texture, 0, 0);
  SDL_RenderPresent(Renderer);

  if (Texture) {
    SDL_DestroyTexture(Texture);
  }

  if (BitmapMemory) {
    free(BitmapMemory);
  }
}
