/** @copyright 2025 Sean Kasun */

#pragma once

#include <SDL3/SDL_gpu.h>
#include <filesystem>
#include <glm/ext/vector_float2.hpp>
#include <unordered_map>

class Textures {
  public:
    bool setPath(const std::filesystem::path &path);
    SDL_GPUTexture *get(SDL_GPUDevice *gpu, SDL_GPUCopyPass *copy, int slot);
    SDL_GPUTexture *flat(SDL_GPUDevice *gpu, SDL_GPUCopyPass *copy, void *data, uint32_t w, uint32_t h);
    glm::vec2 size(int slot);
    void resetFlat(SDL_GPUDevice *gpu);

    enum TextureSlot {
      Tile = 0x1000,
      Wall = 0x2000,
      ArmorHead = 0x3000,
      ArmorBody = 0x4000,
      ArmorLegs = 0x5000,
      TreeTops = 0x6000,
      TreeBranches = 0x7000,
      Xmas = 0x8000,
      Extra = 0x9000,
      Background = 0xa000,
      Liquid = 0xb000,
      LiquidEdge = 0xc000,
      NPC = 0xe000,
      NPCHead = 0xf000,
      Underworld = 0x10000,
      Unique = 0x0000,
      Outline = 0,
      Shroom = 1,
      Actuator = 2,
      Wires = 3,
      Banner = 4,
      Flat = 5,
      Hilite = 6,
    };

  private:
    void load(SDL_GPUDevice *gpu, SDL_GPUCopyPass *copy, int slot, const std::string name);
    std::filesystem::path root;

    std::unordered_map<int, SDL_GPUTexture *>cache;
    std::unordered_map<int, glm::vec2> dims;
};
