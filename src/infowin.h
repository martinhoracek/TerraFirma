/** @copyright 2025 Sean Kasun */

#pragma once

#include "world.h"
#include <vector>

class InfoWin {
  public:
    InfoWin(const World &world);
    void show();

  private:
    void add(const char *key, const char *value);
    struct Row {
      const char *key;
      const char *value;
    };
    std::vector<Row> rows;
};
