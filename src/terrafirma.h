/** @copyright 2025 Sean Kasun */

#pragma once

#include "l10n.h"
#include "settings.h"
#include "map.h"
#include "gui.h"
#include "hilitewin.h"
#include "findchests.h"
#include "infowin.h"
#include "killwin.h"
#include "bestiary.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL.h>
#include <stdlib.h>

class Terrafirma {
  public:
    Terrafirma();
    void init();
    void run();
    void shutdown();

  private:
    void initGui(float scale);
    bool processEvents();
    bool renderGui();
    void shutdownGui();
    void openDialog();
    void openWorld(std::string file);
    void resize();
    void populateWorldMenu();
    void reloadSettings();

    GUI gui;
    Map map;
    World world;
    Settings settings;
    L10n l10n;
    std::string status;
    bool showTextures = true;
    bool canShowTextures = false;
    bool showHouses = false;
    bool showWires = false;
    std::vector<std::filesystem::path> worlds;
    InfoWin *infoWin = nullptr;
    KillWin *killWin = nullptr;
    Bestiary *bestiary = nullptr;
    HiliteWin *hiliteWin = nullptr;
    FindChests *findChests = nullptr;
    std::vector<std::string> viewChest;
    std::string viewSign;

    bool dragging = false;
    bool rightClick = false;
    glm::ivec2 rightClickTile;
    SDL_Thread *loadThread = nullptr;
    SDL_Mutex *loadMutex = nullptr;
    std::string loadError;
    SDL_Thread *searchThread = nullptr;
    SDL_Mutex *searchMutex = nullptr;
};
