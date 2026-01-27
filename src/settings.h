/** @copyright 2025 Sean Kasun */

#pragma once

#include "l10n.h"

#include <vector>
#include <filesystem>

class Settings {
  public:
    Settings();
    std::vector<std::filesystem::path> worldFolders() const;
    std::filesystem::path getTextures() const;
    std::filesystem::path getExe() const;
    std::string getLanguage() const;
    bool show(const L10n &l10n);

  private:
    void init();
    void load();
    void save();
    std::filesystem::path prefFile();
    std::vector<std::filesystem::path> worldDirs;
    std::filesystem::path images;
    std::filesystem::path exe;
    bool autoDetectWorldPath;
    std::string customWorldPath;
    bool autoDetectTextures;
    std::string customTexturesPath;
    bool autoDetectTerraria;
    std::string customTerrariaPath;
    std::string language;
};
