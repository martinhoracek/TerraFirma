/** @copyright 2025 Sean Kasun */

#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <cstdint>
#include "json.h"

class TileInfo {
  public:
    struct MergeBlend {
      bool hasTile;
      int16_t tile;
      uint32_t mask;
      bool blend;
      bool recursive;
      uint8_t direction;
    };
    explicit TileInfo(std::shared_ptr<JSONData> json, const std::unordered_map<uint16_t, std::string> &items);
    TileInfo(std::shared_ptr<JSONData> json, const std::unordered_map<uint16_t, std::string> &items, const TileInfo &parent);
    std::string name;
    uint32_t color;
    double lightR, lightG, lightB;
    uint32_t mask;
    bool solid, transparent, dirt, stone, grass, pile, flip, brick, merge, large;
    std::vector<MergeBlend> blends;
    int width, height, skipy, toppad;
    int u, v, minu, maxu, minv, maxv;
    std::vector<std::shared_ptr<TileInfo>> variants;
};

class WallInfo {
  public:
    WallInfo(std::shared_ptr<JSONData> json, const std::unordered_map<uint16_t, std::string> &items);
    std::string name;
    uint32_t color;
    uint16_t blend;
    uint8_t large = 0;
};

class NPC {
  public:
    explicit NPC(std::shared_ptr<JSONData> json);
    std::string title;
    uint16_t head;
    int16_t id;
};

class WorldInfo {
  public:
    WorldInfo();
    std::shared_ptr<TileInfo> operator[](class Tile const &tile) const;
    std::shared_ptr<TileInfo> operator[](int16_t type) const;
    std::shared_ptr<TileInfo> find(std::shared_ptr<TileInfo> tile, int16_t u, int16_t v) const;

    std::unordered_map<uint16_t, std::string> items;
    std::unordered_map<uint16_t, std::string> prefixes;
    std::unordered_map<int16_t, std::shared_ptr<TileInfo>> tiles;
    std::unordered_map<int16_t, std::shared_ptr<WallInfo>> walls;
    std::unordered_map<uint16_t, std::shared_ptr<NPC>> npcsById;
    std::unordered_map<uint16_t, std::shared_ptr<NPC>> npcsByBanner;
    std::unordered_map<std::string, std::shared_ptr<NPC>> npcsByName;
    uint32_t sky, earth, rock, hell, water, lava, honey, shimmer;
};
