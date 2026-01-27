/** @copyright 2025 Sean Kasun */

#include "hilitewin.h"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <algorithm>
#include <memory>

void lowercase(std::string &a) {
  std::transform(a.begin(), a.end(), a.begin(), [](unsigned char c){ return std::tolower(c);});
}

HiliteWin::HiliteWin(const World &world, const L10n &l10n) {
  for (const auto &tile : world.info.tiles) {
    Block block;
    block.tile = tile.second;
    block.name = l10n.xlateItem(tile.second->name) + " - " + std::to_string(tile.first);
    block.search = block.name;
    lowercase(block.search);
    for (const auto &child : tile.second->variants) {
      if (child->name != tile.second->name && !child->name.empty()) {
        block.children.push_back(addChild(child, l10n));
        block.search += ">" + block.children.back().search;
      }
    }
    std::sort(block.children.begin(), block.children.end(), [](const Block &a, const Block &b) {
                return a.name < b.name;
    });
    blocks.push_back(block);
  }
  std::sort(blocks.begin(), blocks.end(), [](const Block &a, const Block &b) {
              return a.name < b.name;
  });
}

HiliteWin::Block HiliteWin::addChild(std::shared_ptr<TileInfo> tile, const L10n &l10n) {
  Block b;
  b.tile = tile;
  b.name = l10n.xlateItem(tile->name);
  b.search = b.name;
  lowercase(b.search);
  for (const auto &child : tile->variants) {
    if (child->name != tile->name && !child->name.empty()) {
      b.children.push_back(addChild(child, l10n));
      b.search += ">" + b.children.back().search;
    }
  }
  std::sort(b.children.begin(), b.children.end(), [](const Block &a, const Block &b) {
              return a.name < b.name;
  });
  return b;
}

std::shared_ptr<TileInfo> HiliteWin::pickBlock() {
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
    ImGui::SetKeyboardFocusHere(0);
  }
  ImGui::InputText("Search", &search);
  ImGui::BeginChild("##blocks", ImVec2(400, 400));
  for (const auto &block : blocks) {
    auto sel = pickChild(block);
    if (sel) {
      selection = sel;
    }
  }
  ImGui::EndChild();
  if (ImGui::Button("Cancel")) {
    ImGui::CloseCurrentPopup();
  }
  ImGui::SameLine();
  if (ImGui::Button("Okay")) {
    ImGui::CloseCurrentPopup();
    return selection;
  }
  return nullptr;
}

static bool contains(const std::string &haystack, const std::string &needle) {
  auto it = std::search(
    haystack.begin(), haystack.end(),
    needle.begin(), needle.end(),
    [](unsigned char ch1, unsigned char ch2) {
      return std::tolower(ch1) == std::tolower(ch2);
    }
  );
  return it != haystack.end();
}

std::shared_ptr<TileInfo> HiliteWin::pickChild(const Block &block) {
  std::shared_ptr<TileInfo> result = nullptr;
  ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_DefaultOpen;
  if (!search.empty() && !contains(block.search, search)) {
    return result;
  }
  if (block.children.size() == 0) {
    flag |= ImGuiTreeNodeFlags_Leaf;
  }
  if (block.tile == selection) {
    flag |= ImGuiTreeNodeFlags_Selected;
  }
  if (ImGui::TreeNodeEx(block.name.c_str(), flag)) {
    if (block.children.size()) {
      for (const auto &child : block.children) {
        auto sel = pickChild(child);
        if (sel) {
          result = sel;
        }
      }
    } else if (ImGui::IsItemClicked()) {
      result = block.tile;
    }
    ImGui::TreePop();
  }
  return result;
}
