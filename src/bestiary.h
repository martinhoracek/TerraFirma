/** @copyright 2025 Sean Kasun */

#pragma once

#include "world.h"
#include "l10n.h"

class Bestiary {
  public:
    Bestiary(const World &world, const L10n &l10n);
    void show();

  private:
    struct Kill {
      std::string npc;
      int kills;
    };
    std::vector<Kill> kills;
    std::vector<std::string> seen;
    std::vector<std::string> chats;
};
