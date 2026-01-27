/** @copyright 2025 Sean Kasun */

#include "world.h"
#include "handle.h"
#include <string>
#include <vector>
#include <cstring>


bool World::load(const std::string &filename, SDL_Mutex *mutex) {
  loaded = false;
  if (loadLock == nullptr) {
    loadLock = SDL_CreateMutex();
  }
  auto handle = std::make_shared<Handle>(filename);

  auto version = handle->r32();
  setProgress("Loading map version " + std::to_string(version), mutex);
  if (version > MaxVersion) {
    setProgress("Unsupported map version: " + std::to_string(version), mutex);
    return false;
  }
  if (version < MinVersion) {
    setProgress("Map version too old", mutex);
    return false;
  }

  if (version >= 135) {
    auto magic = handle->read(7);
    if (magic != "relogic") {
      setProgress("Not a terraria map file", mutex);
      return false;
    }
    auto type = handle->r8();
    if (type != 2) {
      setProgress("Not a terraria map file", mutex);
      return false;
    }
    handle->skip(4 + 8);  // revision & favorites
  }
  int numSections = handle->r16();
  std::vector<int> sections;
  for (int i = 0; i < numSections; i++) {
    sections.push_back(handle->r32());
  }
  int numTiles = handle->r16();
  uint8_t mask = 0x80;
  uint8_t bits = 0;
  std::vector<bool> extra;
  for (int i = 0; i < numTiles; i++) {
    if (mask == 0x80) {
      bits = handle->r8();
      mask = 1;
    } else {
      mask <<= 1;
    }
    extra.push_back(bits & mask);
  }

  setProgress("Loading header", mutex);
  handle->seek(sections[0]);
  loadHeader(handle, version);
  setProgress("Loading tiles", mutex);
  handle->seek(sections[1]);
  loadTiles(handle, version, extra);
  setProgress("Loading chests", mutex);
  handle->seek(sections[2]);
  loadChests(handle, version);
  setProgress("Loading signs", mutex);
  handle->seek(sections[3]);
  loadSigns(handle);
  setProgress("Loading npcs", mutex);
  handle->seek(sections[4]);
  loadNPCs(handle, version);
  setProgress("Loading entities", mutex);
  handle->seek(sections[5]);
  if (version >= 116) {
    if (version < 122) {
      loadDummies(handle);
    } else {
      loadEntities(handle);
    }
  }
  if (version >= 170) {
    // section 6 is pressure plates
    // we don't need to load them
  }
  if (version >= 189) {
    // section 7 is the town manager
    // it keeps track of npc rooms
    // we don't need it either
  }
  setProgress("Loading bestiary", mutex);
  if (version >= 210) {
    handle->seek(sections[8]);
    loadBestiary(handle);
  }
  if (version >= 220) {
    // section 9 is creative powers
  }

  loaded = true;

  setProgress("Done", mutex);

  // we would spread light here
  return true;
}

void World::setProgress(std::string msg, SDL_Mutex *mutex) {
  SDL_LockMutex(mutex);
  loadProgress = msg;
  SDL_UnlockMutex(mutex);
}

std::string World::progress() {
  return loadProgress;
}

void World::loadHeader(std::shared_ptr<Handle> handle, int version) {
  header.load(handle, version);
  tilesHigh = header["tilesHigh"]->toInt();
  tilesWide = header["tilesWide"]->toInt();

  groundLevel = header["groundLevel"]->toInt();
  rockLevel = header["rockLevel"]->toInt();
  hellLevel = ((tilesHigh - 330) - groundLevel) / 6;
  hellLevel = hellLevel * 6 + groundLevel - 5;

  tiles = new Tile[tilesWide * tilesHigh]();  // () = init to zero
  colors = new uint8_t[tilesWide * tilesHigh * 4];
}

void World::loadTiles(std::shared_ptr<Handle> handle, int version, std::vector<bool> &extra) {
  for (int x = 0; x < tilesWide; x++) {
    int offset = x;
    for (int y = 0; y < tilesHigh; y++) {
      int rle = tiles[offset].load(handle, extra);
      mapColor(tiles[offset], colors + offset * 4, y);  // calculate now so we can take advantage of rle
      int destOffset = offset + tilesWide;
      for (int r = 0; r < rle; r++, destOffset += tilesWide) {
        memcpy(&tiles[destOffset], &tiles[offset], sizeof(Tile));
        memcpy(colors + destOffset * 4, colors + offset * 4, 4);
      }
      y += rle;
      offset = destOffset;
    }
  }
}

void World::loadChests(std::shared_ptr<Handle> handle, int version) {
  chests.clear();
  int numChests = handle->r16();
  int itemsPerChest = version < 294 ? handle->r16() : 0;
  for (int i = 0; i < numChests; i++) {
    Chest chest;
    chest.x = handle->r32();
    chest.y = handle->r32();
    chest.name = handle->rs();
    if (version >= 294) {
      itemsPerChest = handle->r32();
    }
    for (int j = 0; j < itemsPerChest; j++) {
      int stack = handle->r16();
      if (stack > 0) {
        Chest::Item item;
        item.stack = stack;
        item.name = info.items[handle->r32()];
        item.prefix = info.prefixes[handle->r8()];
        chest.items.push_back(item);
      }
    }
    chests.push_back(chest);
  }
}

void World::loadSigns(std::shared_ptr<Handle> handle) {
  signs.clear();

  int numSigns = handle->r16();
  for (int i = 0; i < numSigns; i++) {
    Sign sign;
    sign.text = handle->rs();
    sign.x = handle->r32();
    sign.y = handle->r32();
    signs.push_back(sign);
  }
}

void World::loadNPCs(std::shared_ptr<Handle> handle, int version) {
  npcs.clear();
  shimmered.clear();

  if (version >= 268) {
    int num = handle->r32();
    for (int i = 0; i < num; i++) {
      shimmered[handle->r32()] = true;
    }
  }
  while (handle->r8()) {
    NPC npc;
    npc.head = 0;
    npc.sprite = 0;
    if (version >= 190) {
      npc.sprite = handle->r32();
      if (const auto &child = info.npcsById.find(npc.sprite); child != info.npcsById.end()) {
        npc.head = child->second->head;
        npc.title = child->second->title;
      }
    } else {
      npc.title = handle->rs();
      if (const auto &child = info.npcsByName.find(npc.title); child != info.npcsByName.end()) {
        npc.head = child->second->head;
        npc.sprite = child->second->id;
      }
    }
    npc.name = handle->rs();
    npc.x = handle->rf();
    npc.y = handle->rf();
    npc.homeless = handle->r8();
    npc.homeX = handle->r32();
    npc.homeY = handle->r32();
    if (version >= 213 && handle->r8()) {
      npc.townVariation = handle->r32();
    }
    if (version >= 315) {
      npc.homelessDespawn = handle->r8();
    }
    npcs.push_back(npc);
  }
  if (version >= 140) {
    while (handle->r8()) {
      NPC npc;
      if (version >= 190) {
        npc.sprite = handle->r32();
        if (const auto &child = info.npcsById.find(npc.sprite); child != info.npcsById.end()) {
          npc.title = child->second->title;
        }
      } else {
        npc.title = handle->rs();
        if (const auto &child = info.npcsByName.find(npc.title); child != info.npcsByName.end()) {
          npc.sprite = child->second->id;
        }
      }
      npc.name = "";
      npc.x = handle->rf();
      npc.y = handle->rf();
      npc.homeless = true;
      npcs.push_back(npc);
    }
  }
}

void World::loadDummies(std::shared_ptr<Handle> handle) {
  int numDummies = handle->r32();
  for (int i = 0; i < numDummies; i++) {
    handle->r16();  // x
    handle->r16();  // y
    // we're not going to bother showing this anymore
    // upgrade your terraria.
  }
}

void World::loadEntities(std::shared_ptr<Handle> handle) {
  itemFrames.clear();
  dolls.clear();
  weaponRacks.clear();
  hatRacks.clear();

  int numEntities = handle->r32();
  for (int i = 0; i < numEntities; i++) {
    int type = handle->r8();
    uint32_t id = handle->r32();
    int16_t x = handle->r16();
    int16_t y = handle->r16();
    switch (type) {
      case 0:
        {
          TrainingDummy dummy;
          dummy.id = id;
          dummy.x = x;
          dummy.y = y;
          dummy.npc = handle->r16();
        }
        break;
      case 1:
        {
          ItemFrame frame;
          frame.id = id;
          frame.x = x;
          frame.y = y;
          frame.itemid = handle->r16();
          frame.prefix = handle->r8();
          frame.stack = handle->r16();
          itemFrames.push_back(frame);
        }
        break;
      case 2:
        {
          LogicSensor sensor;
          sensor.id = id;
          sensor.x = x;
          sensor.y = y;
          sensor.type = handle->r8();
          sensor.on = handle->r8() != 0;
        }
        break;
      case 3:
        {
          DisplayDoll doll;
          doll.id = id;
          doll.x = x;
          doll.y = y;
          uint8_t itemPresent = handle->r8();
          uint8_t dyePresent = handle->r8();
          int cnt = 0;
          for (int b = 0; b < 8; b++) {
            if (itemPresent & (1 << b)) {
              doll.armor[cnt++] = handle->r16();
              handle->r8();  // prefix
              handle->r16();  // stack
            }
          }
          cnt = 0;
          for (int b = 0; b < 8; b++) {
            if (dyePresent & (1 << b)) {
              doll.dye[cnt++] = handle->r16();
              handle->r8();  // prefix
              handle->r16();  // stack
            }
          }
          dolls.push_back(doll);
        }
        break;
      case 4:
        {
          WeaponsRack rack;
          rack.id = id;
          rack.x = x;
          rack.y = y;
          rack.item = handle->r16();
          handle->r8();  // prefix
          handle->r16();  // stack
          weaponRacks.push_back(rack);
        }
        break;
      case 5:
        {
          HatRack rack;
          rack.id = id;
          rack.x = x;
          rack.y = y;
          uint8_t present = handle->r8();
          int cnt = 0;
          for (int b = 0; b < 2; b++) {
            if (present & (1 << b)) {
              rack.hats[cnt++] = handle->r16();
              handle->r8();  // prefix
              handle->r16();  // stack
            }
          }
          cnt = 0;
          for (int b = 2; b < 4; b++) {
            if (present & (1 << b)) {
              rack.dyes[cnt++] = handle->r16();
              handle->r8();  // prefix
              handle->r16();  // stack
            }
          }
          hatRacks.push_back(rack);
        }
        break;
      case 6:
        {
          FoodPlatter platter;
          platter.id = id;
          platter.x = x;
          platter.y = y;
          platter.item = handle->r16();
          handle->r8();  // prefix
          handle->r16();  // stack
        }
        break;
    }
  }
}

void World::loadBestiary(std::shared_ptr<Handle> handle) {
  kills.clear();
  int numKills = handle->r32();
  for (int i = 0; i < numKills; i++) {
    auto npc = handle->rs();
    kills[npc] = handle->r32();
  }
  seen.clear();
  int numSights = handle->r32();
  for (int i = 0; i < numSights; i++) {
    seen.push_back(handle->rs());
  }
  chats.clear();
  int numChat = handle->r32();
  for (int i = 0; i < numChat; i++) {
    chats.push_back(handle->rs());
  }
}

void World::mapColor(const Tile &tile, uint8_t *color, int y) {
  uint32_t c = 0;
  if (tile.active()) {
    c = info[tile]->color;
  } else if (tile.wall > 0) {
    c = info.walls[tile.wall]->color;
  } else if (y < groundLevel) {
    c = info.sky;
  } else if (y < rockLevel) {
    c = info.earth;
  } else if (y < hellLevel) {
    c = info.rock;
  } else {
    c = info.hell;
  }
  if (tile.liquid > 0) {
    uint32_t lc = info.water;
    double alpha = 0.5;
    if (tile.shimmer()) {
      alpha = 0.85;
      lc = info.shimmer;
    } else if (tile.honey()) {
      alpha = 0.85;
      lc = info.honey;
    } else if (tile.lava()) {
      alpha = 0.9;
      lc = info.lava;
    }
    double r = (c >> 16) / 255.0;
    double g = ((c >> 8) & 0xff) / 255.0;
    double b = (c & 0xff) / 255.0;
    double lr = (lc >> 16) / 255.0;
    double lg = ((lc >> 8) & 0xff) / 255.0;
    double lb = (lc & 0xff) / 255.0;
    r = lr * alpha + r * (1. - alpha);
    g = lg * alpha + g * (1. - alpha);
    b = lb * alpha + b * (1. - alpha);
    c = ((uint32_t)(r * 255) << 16) |
            ((uint32_t)(g * 255) << 8) |
            (uint32_t)(b * 255);
  }
  *color++ = c >> 16;
  *color++ = (c >> 8) & 0xff;
  *color++ = c & 0xff;
  *color++ = 0xff;
}
