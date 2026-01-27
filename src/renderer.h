/** @copyright 2025 Sean Kasun */

#pragma once

#include "textures.h"
#include "pipelines.h"

#include <filesystem>
#include <SDL3/SDL_gpu.h>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float2.hpp>
#include <unordered_map>
#include <cstdint>
#include <vector>

struct FlatInstance {
  glm::vec2 translate;
  glm::vec2 size;
  glm::vec2 uv;
  glm::vec2 uvsize;
};

struct HiliteInstance {
  glm::vec2 translate;
  glm::vec2 size;
};

struct TileInstance {
  glm::vec2 translate;
  glm::vec2 size;
  glm::vec2 uv;
  uint32_t paint;
  uint32_t slope;
};

struct BackgroundInstance {
  glm::vec2 translate;
  glm::vec2 size;
  glm::vec2 uv;
};

struct LiquidInstance {
  glm::vec2 translate;
  glm::vec2 size;
  glm::vec2 uv;
  float alpha;
};

struct RenderData {
  glm::vec2 uvdims;
  float layer;
  uint32_t offset;
  Pipeline pipeline;
  SDL_GPUSampler *sampler;
  SDL_GPUTexture *tex;
  std::vector<uint32_t> offsets;
};

class Renderer {
  public:
    std::string init(SDL_GPUDevice *gpu);
    bool setTextures(const std::filesystem::path &path);
    void addTile(SDL_GPUCopyPass *copy, int slot, float x, float y, float z, int w, int h, float u, float v, uint8_t paint, bool fliph = false, bool flipv = false);
    void addSlope(SDL_GPUCopyPass *copy, int slot, int slope, float x, float y, float z, int w, int h, float u, float v, uint8_t paint);
    void addHBG(SDL_GPUCopyPass *copy, int slot, float x, float y, float w, float h);
    void addBG(SDL_GPUCopyPass *copy, int slot, float x, float y, float w, float h);
    void addLiquid(SDL_GPUCopyPass *copy, int slot, int x, int y, float z, int w, int h, float v, float alpha);
    void addHouse(SDL_GPUCopyPass *copy, int slot, float x, float y, float z);
    void addFlat(SDL_GPUCopyPass *copy, void *data, float x, float y, float x2, float y2, uint32_t w, uint32_t h);
    void addHilite(SDL_GPUCopyPass *copy, float x, float y, float w, float h);
    void copy(SDL_GPUCopyPass *copy);
    void render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *render, const glm::mat4 &ortho);
    void hiliteBlock(bool hilite);
    void resetFlat();
    void clear();
  private:
    void addGroup(int slot, Pipeline pipeline, SDL_GPUTexture *tex, SDL_GPUSampler *sampler, glm::vec2 size, float z, size_t offset);
    uint32_t copyGroup(SDL_GPUCopyPass *copy, uint8_t *buf, std::shared_ptr<RenderData> group, uint32_t offset);
    void renderGroup(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *render, const glm::mat4 &ortho, std::shared_ptr<RenderData> group);
    SDL_GPUDevice *gpu;
    SDL_GPUTransferBuffer *transfer;
    SDL_GPUSampler *sampler, *bgSampler;
    SDL_GPUBuffer *tiles;
    std::unordered_map<uint16_t, std::shared_ptr<RenderData>> toDraw;
    std::unordered_map<uint16_t, std::shared_ptr<RenderData>> toOverlay;
    std::vector<TileInstance> tileInstances;
    std::vector<BackgroundInstance> backgroundInstances;
    std::vector<LiquidInstance> liquidInstances;
    std::vector<FlatInstance> flatInstances;
    std::vector<HiliteInstance> hiliteInstances;
    Textures textures;
    Pipelines pipelines;
    bool hiliting = false;
};
