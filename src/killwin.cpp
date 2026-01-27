/** @copyright 2025 Sean Kasun */

#include "killwin.h"
#include "imgui.h"

#include <algorithm>

KillWin::KillWin(const World &world, const L10n &l10n) {
  auto list = world.header["killCount"];
  for (int i = 0; i < list->length(); i++) {
    if (world.info.npcsByBanner.contains(i)) {
      std::string name = l10n.xlateNPC(world.info.npcsByBanner.at(i)->title);
      if (!name.empty()) {
        add(name, list->at(i)->toInt());
      }
    }
  }
  std::sort(rows.begin(), rows.end(), [](const Row &a, const Row &b) {
    if (a.kills == b.kills) {
      return a.npc < b.npc;
    }
    return a.kills > b.kills;
  });
}

void KillWin::show() {
  ImGui::SeparatorText("Kills");
  ImGui::BeginChild("##killlist", ImVec2(400, 200));
  if (ImGui::BeginTable("kills", 2)) {
    for (const auto &row : rows) {
      ImGui::TableNextColumn();
      ImGui::Text("%s", row.npc.c_str());
      ImGui::TableNextColumn();
      ImGui::Text("%d", row.kills);
    }
    ImGui::EndTable();
  }
  ImGui::EndChild();
}

void KillWin::add(const std::string &npc, int kills) {
  rows.emplace_back(npc, kills);
}
