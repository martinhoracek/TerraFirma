/** @copyright 2025 Sean Kasun */
#pragma once

#include "SDL3/SDL_mutex.h"
#include "l10n.h"
#include "world.h"
#include "renderer.h"

#include <filesystem>
#include <glm/vec2.hpp>
#include <SDL3/SDL_gpu.h>

class Map {
  public:
    Map(World &world);
    std::string init(SDL_GPUDevice *gpu);
    bool setTextures(const std::filesystem::path &path);
    void setSize(int w, int h);
    bool load(std::string filename, SDL_Mutex *mutex);
    bool loaded();
    std::string progress();
    void copy(SDL_GPUDevice *gpu, SDL_GPUCopyPass *copy);
    void render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *renderPass);
    std::string getStatus(const L10n &l10n, float x, float y);
    void drag(float dx, float dy);
    void scale(float amt);
    void jumpToLocation(float x, float y);
    void jumpToSpawn();
    void jumpToDungeon();
    void npcMenu(const L10n &l10n);
    void showTextures(bool textures);
    void showWires(bool wires);
    void showHouses(bool houses);
    bool hilite(std::shared_ptr<TileInfo> hilite, SDL_Mutex *mutex);
    void stopHilite();
    bool doneSearching();
    glm::ivec2 mouseToTile(float x, float y);

  private:
    void drawTiles(SDL_GPUDevice *gpu, SDL_GPUCopyPass *copy);
    void drawWalls(SDL_GPUDevice *gpu, SDL_GPUCopyPass *copy);
    void drawBackground(SDL_GPUDevice *gpu, SDL_GPUCopyPass *copy);
    void drawLiquids(SDL_GPUDevice *gpu, SDL_GPUCopyPass *copy);
    void drawWires(SDL_GPUDevice *gpu, SDL_GPUCopyPass *copy);
    void drawNPCs(SDL_GPUDevice *gpu, SDL_GPUCopyPass *copy);
    void drawFlat(SDL_GPUDevice *gpu, SDL_GPUCopyPass *copy);
    void drawHilited(SDL_GPUDevice *gpu, SDL_GPUCopyPass *copy);
    int getFoliage(int x, int y, int *variant, int *texw, int *texh);
    int getTreeVariant(int offset);
    int getPalmVariant(int offset);
    int findBranchStyle(int x, int y);
    int wireMask(int x, int y, uint16_t color);
    void calcBounds();
    glm::mat4 project();

    World &world;
    Renderer renderer;
    uint8_t *flat = nullptr;
    int winWidth, winHeight;
    float centerX, centerY, zoom = 1.0;
    int startX = 0, startY = 0, endX = 0, endY = 0;
    bool dirty = true;
    std::vector<glm::vec2> hilited;
    glm::vec2 hiliteSize;
    bool textures;
    bool wires;
    bool houses;
};
