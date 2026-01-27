/** @copyright 2025 Sean Kasun */

#pragma once

#include "world.h"
#include "l10n.h"
#include <string>

class HiliteWin {
  public:
    HiliteWin(const World &world, const L10n &l10n);
    std::shared_ptr<TileInfo> pickBlock();

  private:
    struct Block {
      std::string name;
      std::string search;
      std::vector<Block> children;
      std::shared_ptr<TileInfo> tile;
    };
    Block addChild(std::shared_ptr<TileInfo> tile, const L10n &l10n);
    std::shared_ptr<TileInfo> pickChild(const Block &tile);
    std::vector<Block> blocks;
    std::string search;
    std::shared_ptr<TileInfo> selection = nullptr;
};
