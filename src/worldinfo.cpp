/** @copyright 2025 Sean Kasun */

#include "worldinfo.h"
#include "assets.h"
#include "tiles.h"

#include <SDL3/SDL.h>
#include <memory>
#include <cassert>

static uint32_t readColor(const std::string &s) {
  uint32_t color = 0;
  for (auto c : s) {
    color <<= 4;
    if (c >= '0' && c <= '9') {
      color |= c - '0';
    } else if (c >= 'a' && c <= 'f') {
      color |= c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
      color |= c - 'A' + 10;
    }
  }
  return color;
}

WorldInfo::WorldInfo() {
  try {
    // load items first so later files can reference them
    const auto jitems = JSON::parse(items_json);
    for (int i = 0; i < jitems->length(); i++) {
      const auto &item = jitems->at(i);
      items[item->at("id")->asInt()] = item->at("name")->asString();
    }
    const auto jtiles = JSON::parse(tiles_json);
    for (int i = 0; i < jtiles->length(); i++) {
      const auto &tile = jtiles->at(i);
      tiles[tile->at("id")->asInt()] = std::make_shared<TileInfo>(tile, items);
    }
    const auto jwalls = JSON::parse(walls_json);
    for (int i = 0; i < jwalls->length(); i++) {
      const auto &wall = jwalls->at(i);
      walls[wall->at("id")->asInt()] = std::make_shared<WallInfo>(wall, items);
    }
    const auto jprefixes = JSON::parse(prefixes_json);
    for (int i = 0; i < jprefixes->length(); i++) {
      const auto &prefix = jprefixes->at(i);
      prefixes[prefix->at("id")->asInt()] = prefix->at("name")->asString();
    }
    const auto jnpcs = JSON::parse(npcs_json);
    for (int i = 0; i < jnpcs->length(); i++) {
      const auto &jnpc = jnpcs->at(i);
      const auto npc = std::make_shared<NPC>(jnpc);
      npcsById[jnpc->at("id")->asInt()] = npc;
      if (jnpc->has("banner")) {
        npcsByBanner[jnpc->at("banner")->asInt()] = npc;
      } else if (npcsByName.find(jnpc->at("name")->asString()) == npcsByName.end()) {
        npcsByName[jnpc->at("name")->asString()] = npc;
      } 
    }
    const auto jglobals = JSON::parse(globals_json);
    for (int i = 0; i < jglobals->length(); i++) {
      const auto &global = jglobals->at(i);
      const auto &kind = global->at("id")->asString();
      const auto color = readColor(global->at("color")->asString());
      if (kind == "sky") {
        sky = color;
      } else if (kind == "earth") {
        earth = color;
      } else if (kind == "rock") {
        rock = color;
      } else if (kind == "hell") {
        hell = color;
      } else if (kind == "water") {
        water = color;
      } else if (kind == "lava") {
        lava = color;
      } else if (kind == "honey") {
        honey = color;
      } else if (kind == "shimmer") {
        shimmer = color;
      }
    }
  } catch (JSONParseException e) {
    SDL_Log("Failed: %s", e.reason.c_str());
    exit(-1);
  }
}

std::shared_ptr<TileInfo> WorldInfo::operator[](Tile const &tile) const {
  auto v = tile.v;
  if (tile.type == TileStatues) {
    v %= 162;
  }
  return find(tiles.at(tile.type), tile.u, v);
}

std::shared_ptr<TileInfo> WorldInfo::operator[](int16_t type) const {
  return tiles.at(type);
}

std::shared_ptr<TileInfo> WorldInfo::find(std::shared_ptr<TileInfo> tile, int16_t u, int16_t v) const {
  for (const auto &var : tile->variants) {
    // must match all restrictions
    if ((var->u < 0 || var->u == u) &&
      (var->v < 0 || var->v == v) &&
      (var->minu < 0 || var->minu <= u) &&
      (var->minv < 0 || var->minv <= v) &&
      (var->maxu < 0 || var->maxu > u) &&
      (var->maxv < 0 || var->maxv > v)) {
      return find(var, u, v);  // recursive
    }
  }
  return tile;  // no variants found
}

static TileInfo::MergeBlend parseMB(const std::string &tag, bool blend, int *offset) {
  std::string group = "";
  TileInfo::MergeBlend mb;
  mb.hasTile = false;
  mb.direction = 0;
  mb.mask = 0;
  mb.tile = 0;
  mb.blend = blend;
  mb.recursive = false;
  int i = *offset;
  while (i < tag.length()) {
    char c = tag[i++];
    if (c == ',') {
      break;
    }
    if (c == '*') {
      mb.recursive = true;
    } else if (c == 'v') {
      mb.direction |= 4;
    } else if (c == '^') {
      mb.direction |= 8;
    } else if (c == '+') {
      mb.direction |= 8 | 4 | 2 | 1;
    } else if (c >= '0' && c <= '9') {
      mb.hasTile = true;
      mb.tile *= 10;
      mb.tile += c - '0';
    } else if (c >= 'a' && c <= 'z') {
      group += c;
    } else {
      assert(false && "Unknown type");
    }
  }

  if (mb.direction == 0) {
    mb.direction = 0xff;
  }
  if (!mb.hasTile) {
    if (group == "solid") {
      mb.mask |= 1;
    } else if (group == "dirt") {
      mb.mask |= 4;
    } else if (group == "brick") {
      mb.mask |= 128;
    } else if (group == "moss") {
      mb.mask |= 256;
    } else {
      assert(false && "Unknown group");
    }
  }
  *offset = i;
  return mb;
}

TileInfo::TileInfo(std::shared_ptr<JSONData> json, const std::unordered_map<uint16_t, std::string> &items) {
  static int cnt = 0;
  if (json->has("ref")) {
    if (const auto child = items.find(json->at("ref")->asInt()); child != items.end()) {
      name = child->second;
    }
  } else {
    name = json->at("name")->asString();
  }
  color = json->has("color") ? readColor(json->at("color")->asString()) : 0;
  lightR = json->has("r") ? json->at("r")->asNumber() : 0.0;
  lightG = json->has("g") ? json->at("g")->asNumber() : 0.0;
  lightB = json->has("b") ? json->at("b")->asNumber() : 0.0;
  mask = json->at("flags")->asInt();
  solid = mask & 1;
  transparent = mask & 2;
  dirt = mask & 4;
  stone = mask & 8;
  grass = mask & 0x10;
  pile = mask & 0x20;
  flip = mask & 0x40;
  brick = mask & 0x80;
  //moss = mask & 0x100;
  merge = mask & 0x200;
  large = mask & 0x400;
  u = v = minu = minv = maxu = maxv = 0;

  auto b = json->at("blend")->asString();
  int offset = 0;
  while (offset < b.length()) {
    blends.push_back(parseMB(b, true, &offset));
  }

  auto m = json->at("merge")->asString();
  offset = 0;
  while (offset < m.length()) {
    blends.push_back(parseMB(m, false, &offset));
  }

  width = json->at("w")->asInt(18);
  height = json->at("h")->asInt(18);
  skipy = json->at("skipy")->asInt();
  toppad = json->at("toppad")->asInt();
  if (json->has("var")) {
    const auto &vars = json->at("var");
    for (int i = 0; i < vars->length(); i++) {
      variants.push_back(std::make_shared<TileInfo>(vars->at(i), items, *this));
    }
  }
}

TileInfo::TileInfo(std::shared_ptr<JSONData> json, const std::unordered_map<uint16_t, std::string> &items, const TileInfo &parent) {
  if (json->has("ref")) {
    if (const auto child = items.find(json->at("ref")->asInt()); child != items.end()) {
      name = child->second;
    }
  } else {
    name = json->at("name")->asString();
  }
  if (name.empty()) {
    name = parent.name;
  }
  color = json->has("color") ? readColor(json->at("color")->asString()) : parent.color;
  lightR = json->at("r")->asNumber(parent.lightR);
  lightG = json->at("g")->asNumber(parent.lightG);
  lightB = json->at("b")->asNumber(parent.lightB);

  mask = parent.mask;

  solid = parent.solid;
  transparent = parent.transparent;
  dirt = parent.dirt;
  stone = parent.stone;
  grass = parent.grass;
  pile = parent.pile;
  flip = parent.flip;
  brick = parent.brick;
  merge = parent.merge;
  large = parent.large;

  width = parent.width;
  height = parent.height;
  skipy = parent.skipy;
  toppad = json->at("toppad")->asInt(parent.toppad);
  u = json->at("x")->asInt(-1) * width;
  v = json->at("y")->asInt(-1) * (height + skipy);
  minu = json->at("minx")->asInt(-1) * width;
  maxu = json->at("maxx")->asInt(-1) * width;
  minv = json->at("miny")->asInt(-1) * (height + skipy);
  maxv = json->at("maxy")->asInt(-1) * (height + skipy);

  if (json->has("var")) {
    const auto &vars = json->at("var");
    for (int i = 0; i < vars->length(); i++) {
      variants.push_back(std::make_shared<TileInfo>(vars->at(i), items, *this));
    }
  }
}

WallInfo::WallInfo(std::shared_ptr<JSONData> json, const std::unordered_map<uint16_t, std::string> &items) {
  if (json->has("ref")) {
    if (const auto child = items.find(json->at("ref")->asInt()); child != items.end()) {
      name = child->second;
    }
  } else {
    name = json->at("name")->asString();
  }
  color = json->has("color") ? readColor(json->at("color")->asString()) : 0;
  // blend with itself if it doesn't have a blend set
  blend = json->at("blend")->asInt(json->at("id")->asInt());
}

NPC::NPC(std::shared_ptr<JSONData> json) {
  title = json->at("name")->asString();
  head = json->at("head")->asInt();
  id = json->at("id")->asInt();
}
