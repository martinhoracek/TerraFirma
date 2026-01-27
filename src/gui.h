/** @copyright 2025 Sean Kasun */

#pragma once

#include "map.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#define SDLFAIL() \
  do { \
    SDL_Log("%s:%d %s\n", __FILE__, __LINE__, SDL_GetError()); \
    abort(); \
  } while (0);

#define FAIL(...) \
  do { \
    SDL_Log("%s:%d ", __FILE__, __LINE__); \
    SDL_Log(__VA_ARGS__); \
    abort(); \
  } while (0)

class GUI {
  public:
    SDL_GPUDevice *init();
    void resizeSwapchain(Map *map);
    bool processEvents(SDL_Event *event);
    bool fence();
    void render(Map *map);
    void shutdown();

  private:
    void initImgui(float scale);
    SDL_Window *window;
    SDL_GPUDevice *gpu;
    SDL_GPUFence *renderFence = nullptr;
    SDL_GPUTexture *drawImage, *depthImage;
    int winWidth, winHeight;
};
