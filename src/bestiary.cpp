/** @copyright 2025 Sean Kasun */

#include "bestiary.h"
#include "imgui.h"

#include <algorithm>

Bestiary::Bestiary(const World &world, const L10n &l10n) {
  for (const auto &k : world.kills) {
    Kill kill;
    kill.npc = l10n.xlateNPC(k.first);
    kill.kills = k.second;
    kills.push_back(kill);
  }
  std::sort(kills.begin(), kills.end(), [](const Kill &a, const Kill &b) {
    if (a.kills == b.kills) {
      return a.npc < b.npc;
    }
    return a.kills > b.kills;
  });
  for (const auto &s : world.seen) {
    seen.push_back(l10n.xlateNPC(s));
  }
  for (const auto &c : world.chats) {
    chats.push_back(l10n.xlateNPC(c));
  }
}

void Bestiary::show() {
  ImGuiStyle &style = ImGui::GetStyle();
  ImGui::PushFont(NULL, style.FontSizeBase * 1.5);
  ImGui::Text("Bestiary");
  ImGui::PopFont();
  ImGui::SeparatorText("Kills");
  ImGui::BeginChild("##killlist", ImVec2(400, 200));
  if (ImGui::BeginTable("kills", 2)) {
    for (const auto &row : kills) {
      ImGui::TableNextColumn();
      ImGui::Text("%s", row.npc.c_str());
      ImGui::TableNextColumn();
      ImGui::Text("%d", row.kills);
    }
    ImGui::EndTable();
  }
  ImGui::EndChild();
  ImGui::SetNextItemWidth(200);
  ImGui::SeparatorText("Seen");
  ImGui::SameLine(200);
  ImGui::SetNextItemWidth(200);
  ImGui::SeparatorText("Chat");
  
  ImGui::BeginChild("##seenlist", ImVec2(200, 200));
  for (const auto &row : seen) {
    ImGui::Text("%s", row.c_str());
  }
  ImGui::EndChild();
  ImGui::SameLine();
  ImGui::BeginChild("##chatlist", ImVec2(200, 200));
  for (const auto &row : chats) {
    ImGui::Text("%s", row.c_str());
  }
  ImGui::EndChild();
}
