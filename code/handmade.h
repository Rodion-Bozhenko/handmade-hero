#ifndef HANDMADEH_H
#define HANDMADEH_H

struct GameOffscreenBuffer {
  void *memory;
  int width;
  int height;
  int pitch;
  int bytesPerPixel;
};

void gameUpdateAndRender(GameOffscreenBuffer *buffer, int blueOffset,
                         int greenOffset);

#endif
