/** @copyright 2025 Sean Kasun */

#pragma once

#include "world.h"
#include "l10n.h"

#include <vector>
#include <glm/ext/vector_float2.hpp>

class FindChests {
  public:
    FindChests(const World &world, const L10n &l10n);
    glm::vec2 pickChest();

  private:
    struct Chest {
      std::string name;
      glm::vec2 location;
    };
    struct Item {
      std::string name;
      std::vector<Chest> chests;
      std::set<std::pair<float, float>> seen;
    };
    std::string search;
    std::vector<Item> items;
    glm::vec2 selected;
};
