/** @copyright 2025 Sean Kasun */

#pragma once

#include "world.h"
#include "l10n.h"

#include <vector>

class KillWin {
  public:
    KillWin(const World &world, const L10n &l10n);
    void show();

  private:
    void add(const std::string &npc, int kills);
    struct Row {
      std::string npc;
      int kills;
    };
    std::vector<Row> rows;
};
