/** @copyright 2025 Sean Kasun */

/*
 * This crazy chunk of code parses a .Net executable and extracts the
 * managed resources from it.  We need this to get the .json translation
 * files from terraria.
 */

#include "l10n.h"
#include "handle.h"
#include <SDL3/SDL.h>
#include <regex>

enum {
  TILDE = 0,
  STRINGS = 1,
};

struct Stream {
  uint32_t offset, size;
};

struct Resource {
  Resource(uint32_t name, uint32_t offset) : name(name), offset(offset) {}
  uint32_t name, offset;
};

static uint32_t fits0(uint32_t v) {
  return v > 0xffff ? 4 : 2;
}
static uint32_t fits1(uint32_t a, uint32_t b) {
  return (a | b) > 0x7fff ? 4 : 2;
}
static uint32_t fits2(uint32_t a, uint32_t b, uint32_t c, uint32_t d = 0) {
  return (a | b | c | d) > 0x3fff ? 4 : 2;
}
static uint32_t fits3(uint32_t a, uint32_t b, uint32_t c = 0, uint32_t d = 0, uint32_t e = 0) {
  return (a | b | c | d | e) > 0x1fff ? 4 : 2;
}
static uint32_t fits5(uint32_t a) {
  return a > 0x7ff ? 4 : 2;
}

void L10n::load(std::string exe) {
  Handle handle(exe);
  if (!handle.isOpen()) {
    return;
  }
  if (handle.r16() != 0x5a4d) {  // not an MZ exe
    return;
  }
  handle.seek(0x3c);
  handle.seek(handle.r32());
  if (handle.r32() != 0x4550) {  // not a PE exe
    return;
  }
  handle.skip(2);
  auto numSections = handle.r16();
  handle.skip(12);
  auto headerLen = handle.r16();
  handle.skip(headerLen + 2);
  bool found = false;
  uint32_t base, offset;
  for (int i = 0; i < numSections; i++) {
    if (handle.read(5) == ".text") {
      handle.skip(7);
      base = handle.r32();
      handle.skip(4);
      offset = handle.r32();
      found = true;
      break;
    }
    handle.skip(35);
  }
  if (!found) {  // doesn't contain a .text segment
    return;
  }
  handle.seek(offset + 0x10);
  auto metaRVA = handle.r32();
  handle.skip(12);
  auto resourceRVA = handle.r32();

  handle.seek(metaRVA + offset - base + 0xc);
  auto verLen = handle.r32();
  handle.skip(verLen + 2);
  Stream streams[2];
  auto numStreams = handle.r16();
  for (int i = 0; i < numStreams; i++) {
    auto strOfs = handle.r32();
    auto strSize = handle.r32();
    int type = -1;
    std::string tname = handle.rcs();
    while (handle.tell() & 3) {
      handle.skip(1);
    }
    if (tname == "#~") {
      type = TILDE;
    } else if (tname == "#Strings") {
      type = STRINGS;
    }
    if (type >= 0) {
      streams[type].offset = strOfs;
      streams[type].size = strSize;
    }
  }
  handle.seek(metaRVA + streams[TILDE].offset + offset - base + 6);
  auto indexWidths = handle.r16();
  int strWidth = (indexWidths & 1) ? 4 : 2;
  int guidWidth = (indexWidths & 2) ? 4 : 2;
  int blobWidth = (indexWidths & 4) ? 4 : 2;
  auto tables = handle.r64();
  handle.skip(8);
  uint32_t rows[64];
  for (int i = 0; i < 64; i++) {
    if (tables & 1) {
      rows[i] = handle.r32();
    } else {
      rows[i] = 0;
    }
    tables >>= 1;
  }

  const int TypeDefOrRef = fits2(rows[1], rows[2], rows[27]);
  const int MethodDefOrRef = fits1(rows[6], rows[10]);
  uint32_t CustomAttr = rows[0];
  int customRows[] = {
    1, 2, 4, 6, 8, 9, 10, 17, 20, 23, 26, 27, 32, 35, 38, 39, 40
  };
  for (int i = 0; i < SDL_arraysize(customRows); i++) {
    if (rows[customRows[i]] > CustomAttr) {
      CustomAttr = rows[customRows[i]];
    }
  }
  /*
   * Ugh, in order to seek into the stream, you need to handle all the types
   * in order.  Since we want resources, which is type 40, we need to
   * calculate the sizes of types 0 - 39. */
  uint32_t skip =
      // Module
      rows[0] * (2 + strWidth + guidWidth * 3) +
      // TypeRef
      rows[1] * (fits2(rows[0], rows[1], rows[26], rows[35]) + strWidth * 2) +
      // TypeDef
      rows[2] * (4 + strWidth * 2 + TypeDefOrRef + fits0(rows[4]) + fits0(rows[6])) +
      // Field
      rows[4] * (2 + strWidth + blobWidth) +
      // MehtodDef
      rows[6] * (8 + strWidth + blobWidth + fits0(rows[8])) +
      // Param
      rows[8] * (4 + strWidth) +
      // InterfaceImpl
      rows[9] * (fits0(rows[2]) + TypeDefOrRef) +
      // MemberRef
      rows[10] * (fits3(rows[1], rows[2], rows[6], rows[26],rows[27]) +
      strWidth + blobWidth) +
      // Constant
      rows[11] * (2 + fits2(rows[4], rows[8], rows[23]) + blobWidth) +
      // CustomAttribute
      rows[12] * (fits5(CustomAttr) + fits3(rows[6], rows[10]) + blobWidth) +
      // FieldMarshal
      rows[13] * (fits1(rows[4], rows[8]) + blobWidth) +
      // DeclSecurity
      rows[14] * (2 + fits2(rows[2], rows[6], rows[32]) + blobWidth) +
      // ClassLayout
      rows[15] * (6 + fits0(rows[2])) +
      // FieldLayout
      rows[16] * (4 + fits0(rows[4])) +
      // StandAloneSig
      rows[17] * blobWidth +
      // EventMap
      rows[18] * (fits0(rows[2]) + fits0(rows[20])) +
      // Event
      rows[20] * (2 + strWidth + TypeDefOrRef) +
      // PropertyMap
      rows[21] * (fits0(rows[2]) + fits0(rows[23])) +
      // Property
      rows[23] * (2 + strWidth + blobWidth) +
      // MethodSemantics
      rows[24] * (2 + fits0(rows[6]) + fits1(rows[20], rows[23])) +
      // MethodImpl
      rows[25] * (fits0(rows[2]) + MethodDefOrRef * 2) +
      // ModuleRef
      rows[26] * strWidth +
      // TypeSpec
      rows[27] * blobWidth +
      // ImplMap
      rows[28] * (2 + fits1(rows[4], rows[6]) + strWidth + fits0(rows[26])) +
      // FieldRVA
      rows[29] * (4 + fits0(rows[4])) +
      // Assembly
      rows[32] * (16 + blobWidth + strWidth * 2) +
      // AssemblyProcessor
      rows[33] * 4 +
      // AssemblyOS
      rows[34] * 12 +
      // AssemblyRef
      rows[35] * (12 + blobWidth * 2 + strWidth * 2) +
      // AssemblyRefProcessor
      rows[36] * (4 + fits0(rows[35])) +
      // AssemblyRefOS
      rows[37] * (12 + fits0(rows[35])) +
      // File
      rows[38] * (4 + strWidth + blobWidth) +
      // ExportedType
      rows[39] * (8 + strWidth * 2 + fits2(rows[35], rows[38], rows[39]));
  handle.skip(skip);
  uint32_t resLen = fits2(rows[35], rows[38], rows[39]);
  std::vector<std::shared_ptr<Resource>> resources;
  for (int i = 0; i < rows[40]; i++) {
    uint32_t ofs = handle.r32();
    handle.skip(4);
    uint32_t name = strWidth == 4 ? handle.r32() : handle.r16();
    handle.skip(resLen);
    resources.push_back(std::make_shared<Resource>(name, ofs));
  }
  handle.seek(metaRVA + streams[STRINGS].offset + offset - base);
  std::regex re("Terraria\\.Localization\\.Content\\.([^.]+)\\.([^.]+)\\.json");
  for (const auto &r : resources) {
    handle.seek(metaRVA + streams[STRINGS].offset + offset - base + r->name);
    auto name = handle.rcs();
    std::cmatch match;
    if (std::regex_search(name.c_str(), match, re)) {
      if (match[2] == "Items" || match[2] == "NPCs") {
        std::string lang = match[1].str();
        languages.insert(lang);
        if (lang == currentLanguage) {
          handle.seek(r->offset + resourceRVA + offset - base);
          auto len = handle.r32();

          std::string raw = handle.read(len);
          std::regex comma(",\\s*\\}");
          raw = std::regex_replace(raw, comma, "}");  // remove trailing commas
          auto doc = JSON::parse(raw);
          if (doc) {
            if (match[2] == "Items") {
              items.emplace(lang, doc->at("ItemName"));
              prefixes.emplace(lang, doc->at("Prefix"));
            } else if (match[2] == "NPCs") {
              npcs.emplace(lang, doc->at("NPCName"));
            }
          }
        }
      }
    }
  }
}

void L10n::setLanguage(std::string lang) {
  currentLanguage = lang;
}

std::string L10n::selectedLanguage() const {
  return currentLanguage;
}

std::vector<std::string> L10n::getLanguages() const {
  std::vector<std::string> v;
  v.assign(languages.begin(), languages.end());
  return v;
}

std::string L10n::xlateItem(const std::string &key) const {
  auto json = items.find(currentLanguage);
  if (json == items.end()) {
    return key;
  }
  auto str = json->second->at(key)->asString();
  std::regex re("\\{\\$ItemName\\.(.+?)\\}");
  std::cmatch match;
  if (std::regex_search(str.c_str(), match, re)) {
    str = std::regex_replace(str, re, xlateItem(match[1]));
  }
  if (str.length() == 0) {
    return key;
  }
  return str;
}

std::string L10n::xlatePrefix(const std::string &key) const {
  auto json = prefixes.find(currentLanguage);
  if (json == prefixes.end()) {
    return key;
  }
  return json->second->at(key)->asString();
}

std::string L10n::xlateNPC(const std::string &key) const {
  auto json = npcs.find(currentLanguage);
  if (json == npcs.end()) {
    return key;
  }
  auto str = json->second->at(key)->asString();
  std::regex re("\\{\\$NPCName\\.(.+?)\\}");
  std::cmatch match;
  if (std::regex_search(str.c_str(), match, re)) {
    str = std::regex_replace(str, re, xlateNPC(match[1]));
  }
  return str;
}
