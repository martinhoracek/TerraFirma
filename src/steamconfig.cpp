/** @copyright 2025 Sean Kasun */

#include "steamconfig.h"
#include <cctype>
#include <fstream>
#include <filesystem>
#include <unistd.h>
#include <pwd.h>
#include <cctype>
#include <algorithm>

static const char *libFolders[] = {
  "~/.local/share/Steam",   // linux
  "~/Library/Application Support/Steam",  // mac
  "C:/Program Files (x86)/Steam",  // windows
  nullptr,
};

SteamConfig::SteamConfig() {
  std::string home = getpwuid(getuid())->pw_dir;
  for (int i = 0; libFolders[i]; i++) {
    auto base = expand(libFolders[i]);
    auto vdf = parsevdf(base / "config" / "libraryfolders.vdf");
    if (vdf != nullptr) {
      steamBase = base;
      for (const auto &lib : vdf->children) {
        // check if terraria is installed in this library
        if (!lib.second.find("apps/105600").empty()) {
          std::filesystem::path appbase = lib.second.find("path");
          auto acf = parsevdf(appbase / "steamapps" / "appmanifest_105600.acf");
          if (acf != nullptr) {
            terrariaBase = appbase / "steamapps" / "common" / acf->find("installdir");
          }
        }
      }
    }
  }
}

std::filesystem::path SteamConfig::expand(const char *path) const {
  if (path[0] == '~') {
#ifdef WIN32
    PWSTR docpath;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &docpath);
    if (SUCCEEDED(hr)) {
      std::filesystem::path home = docpath;
      CoTaskMemFree(docpath);
      return home + (path + 1);
    }
#else
    std::string home = getpwuid(getuid())->pw_dir;
    return home + (path + 1);
#endif
  }
  return path;
}

std::filesystem::path SteamConfig::getBase() const {
  return steamBase;
}

std::filesystem::path SteamConfig::getTerraria() const {
  return terrariaBase;
}

std::unique_ptr<SteamConfig::Element> SteamConfig::parsevdf(const std::filesystem::path &filename) {
  std::ifstream f(filename, std::ios::in | std::ios::binary);
  if (f.is_open()) {
    const auto sz = std::filesystem::file_size(filename);
    std::string data(sz, '\0');
    f.read(data.data(), sz);
    f.close();
    Tokenizer t { data, 0 };
    if (t.next() == '"') {
      return std::make_unique<Element>(&t);
    }
  }
  return nullptr;
}

SteamConfig::Element::Element() = default;


SteamConfig::Element::Element(Tokenizer *t) {
  name = t->key();
  if (name.empty()) {  // failed?!
    return;
  }
  std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){ return std::tolower(c); });
  switch (t->next()) {
    case '"':  // string value
      value = t->key();
      break;
    case '{':  // children
      while (t->next() == '"') {
        Element e(t);
        children[e.name] = e;
      }
      break;
  }
}

char SteamConfig::Tokenizer::next() {
  pos = data.find_first_of("\"{}", pos);
  if (pos == data.npos) {
    return 0;
  }
  return data[pos++];
} 

std::string SteamConfig::Tokenizer::key() {
  size_t start = pos;
  // find trailing quote
  while ((pos = data.find_first_of('"', pos)) != data.npos && data[pos - 1] == '\\') {
    pos++;
  }
  if (pos != data.npos) {
    pos++;
    return data.substr(start, pos - 1 - start);
  }
  return "";
}

std::string SteamConfig::Element::find(const std::string &path) const {
  size_t ofs = path.find_first_of('/');
  if (ofs == path.npos) {
    if (auto child = children.find(path); child != children.end()) {
      return child->second.value;
    }
  } else if (auto child = children.find(path.substr(0, ofs)); child != children.end()) {
    return child->second.find(path.substr(ofs + 1));
  }
  return "";
}
