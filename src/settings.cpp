/** @copyright 2025 Sean Kasun */

#include "settings.h"
#include "json.h"
#include "steamconfig.h"
#include "filedialogfont.h"
#include "handle.h"
#include "gui.h"

#include <SDL3/SDL_filesystem.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <SDL3/SDL.h>
#include <ImGuiFileDialog.h>

#include <filesystem>
#include <vector>

static const char *docFolders[] = {
  "~/.local/share/Terraria",  // linux
  "~/Library/Application Support/Terraria",   // mac
  "~/My Games/Terraria",  // windows
  nullptr
};

Settings::Settings() {
  load();  // load before init?  load gets settings on whether or not to search for folders in init
  init();
}

void Settings::init() {
  SteamConfig steam;
  std::filesystem::path base = steam.getBase();

  worldDirs.clear();

  std::vector<std::filesystem::path> dirs;

  if (autoDetectWorldPath) {
    for (int i = 0; docFolders[i]; i++) {
      const auto dir = steam.expand(docFolders[i]);
      if (std::filesystem::is_directory(dir)) {
        dirs.push_back(dir);
      }
    }

    // check for remote worlds in steam
    const auto userpath = base / "userdata";
    if (std::filesystem::is_directory(userpath)) {
      const std::filesystem::directory_iterator userdata(userpath);
      for (const auto &file : userdata) {
        const auto dir = file.path() / "105600" / "remote";
        const auto wdir = dir / "worlds";
        if (std::filesystem::is_directory(wdir)) {
          worldDirs.push_back(wdir);
        }
      }
    }

    // look through the standard folders
    for (const auto &dir : dirs) {
      const auto wdir = dir / "Worlds";
      if (std::filesystem::is_directory(wdir)) {
        worldDirs.push_back(wdir);
      }
    }
  } else {
    worldDirs.push_back(customWorldPath);
  }

  if (autoDetectTextures) {
    images = steam.getTerraria() / "Content" / "Images";
    if (!std::filesystem::is_directory(images)) {
      // try the mac path
      images = steam.getTerraria() / "Terraria.app" / "Contents" / "Resources" / "Content" / "Images";
    }
  } else {
    images = customTexturesPath;
  }

  if (autoDetectTerraria) {
    exe = steam.getTerraria() / "Terraria.exe";
    if (!std::filesystem::is_regular_file(exe)) {
      // try the mac path
      exe = steam.getTerraria() / "Terraria.app" / "Contents" / "Resources" / "Terraria.exe";
    }
  } else {
    exe = customTerrariaPath;
  }
}

std::vector<std::filesystem::path> Settings::worldFolders() const {
  return worldDirs;
}

std::filesystem::path Settings::getTextures() const {
  return images;  
}

std::filesystem::path Settings::getExe() const {
  return exe;
}

std::string Settings::getLanguage() const {
  return language;
}

bool Settings::show(const L10n &l10n) {
  IGFD::FileDialogConfig config {
    .path = ".",
    .countSelectionMax = 1,
    .flags = ImGuiFileDialogFlags_Modal,
  };
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir, nullptr, ImVec4(0.5f, 1.0f, 0.9f, 0.9f), (const char *)ICON_IGFD_FOLDER);
  ImGui::Checkbox("Auto-detect World Folder", &autoDetectWorldPath);
  if (!autoDetectWorldPath) {
    ImGui::InputText("World Path", &customWorldPath);
    ImGui::SameLine();
    if (ImGui::Button((const char *)ICON_IGFD_FOLDER_OPEN "##worlds")) {
      ImGuiFileDialog::Instance()->OpenDialog("chooseWorlds", "Choose World Folder", nullptr, config);
    }
  }
  ImGui::Checkbox("Auto-detect Texture Folder", &autoDetectTextures);
  if (!autoDetectTextures) {
    ImGui::InputText("Texture Path", &customTexturesPath);
    ImGui::SameLine();
    if (ImGui::Button((const char *)ICON_IGFD_FOLDER_OPEN "##textures")) {
      ImGuiFileDialog::Instance()->OpenDialog("chooseTextures", "Choose Textures Folder", nullptr, config);
    }
  }
  ImGui::Checkbox("Auto-detect Terraria Executable", &autoDetectTerraria);
  if (!autoDetectTerraria) {
    ImGui::InputText("Terraria Path", &customTerrariaPath);
    ImGui::SameLine();
    if (ImGui::Button((const char *)ICON_IGFD_FOLDER_OPEN "##terraria")) {
      ImGuiFileDialog::Instance()->OpenDialog("chooseTerraria", "Choose Terraria.exe", "terraria.exe", config);
    }
  }
  if (ImGui::BeginCombo("Language", language.c_str(), ImGuiComboFlags_HeightRegular)) {
    for (const auto &l : l10n.getLanguages()) {
      const bool sel = l == language;
      if (ImGui::Selectable(l.c_str(), sel)) {
        language = l;
      }
      if (sel) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  bool update = false;
  if (ImGui::Button("Okay")) {
    save();
    init();
    update = true;
    ImGui::CloseCurrentPopup();
  }
  
  if (ImGuiFileDialog::Instance()->Display("chooseWorlds", ImGuiWindowFlags_NoCollapse, ImVec2(600, 400))) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      customWorldPath = ImGuiFileDialog::Instance()->GetCurrentPath();
    }
    ImGuiFileDialog::Instance()->Close();
  }
  if (ImGuiFileDialog::Instance()->Display("chooseTextures", ImGuiWindowFlags_NoCollapse, ImVec2(600, 400))) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      customTexturesPath = ImGuiFileDialog::Instance()->GetCurrentPath();
    }
    ImGuiFileDialog::Instance()->Close();
  }
  if (ImGuiFileDialog::Instance()->Display("chooseTerraria", ImGuiWindowFlags_NoCollapse, ImVec2(600, 400))) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      customTerrariaPath = ImGuiFileDialog::Instance()->GetFilePathName();
    }
    ImGuiFileDialog::Instance()->Close();
  }
  return update;
}

std::filesystem::path Settings::prefFile() {
  char *prefdir = SDL_GetPrefPath("seancode", "terrafirma");
  std::filesystem::path dir = prefdir;
  SDL_free(prefdir);
  return dir / "settings.json";
}

static const char *defaultSavesKey = "use_default_save_path";
static const char *pathToSavesKey = "path_to_saves";
static const char *defaultTexturesKey = "use_default_texture_path";
static const char *pathToTexturesKey = "path_to_textures";
static const char *defaultTerrariaKey = "use_default_terraria_path";
static const char *pathToTerrariaKey = "path_to_terraria";
static const char *languageKey = "language";

void Settings::load() {
  // defaults
  autoDetectWorldPath = true;
  customWorldPath[0] = 0;
  autoDetectTextures = true;
  customTexturesPath[0] = 0;
  autoDetectTerraria = true;
  customTerrariaPath[0] = 0;
  language = "en-US";

  Handle h(prefFile().string());
  if (h.isOpen()) {
    try {
      auto data = JSON::parse(h.read(h.length));
      autoDetectWorldPath = data->at(defaultSavesKey)->asBool();
      customWorldPath = data->at(pathToSavesKey)->asString();
      autoDetectTextures = data->at(defaultTexturesKey)->asBool();
      customTexturesPath = data->at(pathToTexturesKey)->asString();
      autoDetectTerraria = data->at(defaultTerrariaKey)->asBool();
      customTerrariaPath = data->at(pathToTerrariaKey)->asString();
      language = data->at(languageKey)->asString();
    } catch (JSONParseException e) {
      FAIL("Corrupted preferences: %s", e.reason.c_str());
    }
  }
}

static std::string quote(std::string input) {
  static const char *hex = "0123456789abcdef";
  std::string r;
  r += '"';
  for (char ch : input) {
    if (ch >= ' ' && ch <= '~' && ch != '\\' && ch != '"') {
      r += ch;
    } else {
      r += '\\';
      switch (ch) {
        case '"':
          r += '"';
          break;
        case '\\':
          r += '\\';
          break;
        case '\t':
          r += '\t';
          break;
        case '\r':
          r += '\r';
          break;
        case '\n':
          r += '\n';
          break;
        default:
          r += 'x';
          r += hex[ch >> 4];
          r += hex[ch & 0xf];
          break;
      }
    }
  }
  r += '"';
  return r;
}

void Settings::save() {
  std::ofstream f(prefFile(), std::ios::out);
  if (!f.is_open()) {
    return;
  }
  std::string out = "{\n" +
                    quote(defaultSavesKey) + ":" + (autoDetectWorldPath ? "true" : "false") + ",\n" +
                    quote(pathToSavesKey) + ":" + quote(customWorldPath) + ",\n" +
                    quote(defaultTexturesKey) + ":" + (autoDetectTextures ? "true" : "false") + ",\n" +
                    quote(pathToTexturesKey) + ":" + quote(customTexturesPath) + ",\n" +
                    quote(defaultTerrariaKey) + ":" + (autoDetectTerraria ? "true" : "false") + ",\n" +
                    quote(pathToTerrariaKey) + ":" + quote(customTerrariaPath) + ",\n" +
                    quote(languageKey) + ":" + quote(language) + "\n" +
                    "}\n";
  f.write(out.c_str(), out.length());
  f.close();
}
