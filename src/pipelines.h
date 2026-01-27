/** @copyright 2025 Sean Kasun */

#pragma once

#include <string>
#include <unordered_map>
#include <SDL3/SDL_gpu.h>

enum class Pipeline {
  Tile, Background, Liquid, Flat, Hilite
};

struct ShaderSource {
  const uint8_t *spv, *msl, *dxil;
  size_t spvSize, mslSize, dxilSize;
};

class Pipelines {
  public:
    std::string init(SDL_GPUDevice *gpu);
    SDL_GPUGraphicsPipeline *get(Pipeline id);

  private:
    SDL_GPUShader *loadShader(SDL_GPUDevice *gpu, const ShaderSource &source,
                              const SDL_GPUShaderCreateInfo &createInfo);
    std::unordered_map<Pipeline, SDL_GPUGraphicsPipeline *> pipelines;
};

