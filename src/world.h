/** @copyright 2025 Sean Kasun */

#pragma once

#include "SDL3/SDL_mutex.h"
#include "handle.h"
#include "worldheader.h"
#include "worldinfo.h"
#include "tiles.h"

class World {
  public:
    bool load(const std::string &filename, SDL_Mutex *mutex);
    std::string progress();
    int tilesWide, tilesHigh;
    WorldInfo info;
    WorldHeader header;
    Tile *tiles;
    uint8_t *colors;
    bool loaded = false;
    bool failed = false;

    struct Chest {
      struct Item {
        int16_t stack;
        std::string name;
        std::string prefix;
      };
      int32_t x, y;
      std::string name;
      std::vector<Item> items;
    };

    struct Sign {
      int32_t x, y;
      std::string text;
    };

    struct NPC {
      std::string title;
      std::string name;
      float x, y;
      bool homeless, homelessDespawn;
      int32_t homeX, homeY;
      int32_t townVariation;
      int16_t sprite;
      int16_t head;
      int16_t order;
    };

    struct Entity {
      int32_t id;
      int16_t x, y;
    };

    struct TrainingDummy : Entity {
      int16_t npc;
    };

    struct ItemFrame : Entity {
      int16_t itemid;
      uint8_t prefix;
      int16_t stack;
    };

    struct LogicSensor : Entity {
      int8_t type;
      bool on;
    };

    struct DisplayDoll : Entity {
      uint16_t armor[8];
      uint16_t dye[8];
    };

    struct WeaponsRack : Entity {
      uint16_t item;
    };

    struct HatRack : Entity {
      uint16_t hats[2];
      uint16_t dyes[2];
    };

    struct FoodPlatter : Entity {
      uint16_t item;
    };

    std::vector<DisplayDoll> dolls;
    std::vector<NPC> npcs;
    std::vector<Chest> chests;
    std::vector<Sign> signs;
    std::unordered_map<std::string, int32_t> kills;
    std::vector<std::string> seen;
    std::vector<std::string> chats;

  private:
    void loadHeader(std::shared_ptr<Handle> handle, int version);
    void loadTiles(std::shared_ptr<Handle> handle, int version, std::vector<bool> &extra);
    void loadChests(std::shared_ptr<Handle> handle, int version);
    void loadSigns(std::shared_ptr<Handle> handle);
    void loadNPCs(std::shared_ptr<Handle> handle, int version);
    void loadDummies(std::shared_ptr<Handle> handle);
    void loadEntities(std::shared_ptr<Handle> handle);
    void loadBestiary(std::shared_ptr<Handle> handle);
    void mapColor(const Tile &tile, uint8_t *color, int y);
    void render();
    void setProgress(std::string msg, SDL_Mutex *mutex);

    std::vector<ItemFrame> itemFrames;
    std::vector<HatRack> hatRacks;
    std::vector<WeaponsRack> weaponRacks;
    std::unordered_map<uint32_t, bool> shimmered;

    int groundLevel, rockLevel, hellLevel;

    std::string player;
    SDL_Mutex *loadLock;
    std::string loadProgress;
};

