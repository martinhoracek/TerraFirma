/** @copyright 2025 Sean Kasun */

#pragma once

#include <cstdint>

class UVRules {
  public:
    static uint8_t mapTile(const class World &world, int x, int y);
    static void mapWall(const class World &world, int x, int y);
    static void mapCactus(const class World &world, int x, int y);
};
