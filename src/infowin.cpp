/** @copyright 2025 Sean Kasun */

#include "infowin.h"
#include "imgui.h"

InfoWin::InfoWin(const World &world) {
  const char *on = "☑";
  const char *off = "☐";
  const auto &h = world.header;
  add("World Type", h.is("crimson") ? "Crimson" : "Corruption");
  add("Game Mode", h.is("lunarApocalypse") ? "Lunar" : h.is("hardMode") ? "Hard" : "Normal");
  const char *modes[] = {
    "Normal",
    "Expert",
    "Master",
    "Journey",
  };
  add("World Mode", h.is("master") ? "Master" : h.is("expert") ? "Expert" : modes[h["gameMode"]->toInt()]);
  add("Saved Angler", h.is("savedAngler") ? on : off);
  add("Saved Mechanic", h.is("savedMechanic") ? on : off);
  add("Saved Tinkerer", h.is("savedTinkerer") ? on : off);
  add("Saved Stylist", h.is("savedStylist") ? on : off);
  add("Saved Tax Collector", h.is("savedTaxCollector") ? on : off);
  add("Saved Bartender", h.is("savedBartender") ? on : off);
  add("Saved Wizard", h.is("savedWizard") ? on : off);
  add("King Slime", h.is("killedSlimeKing") ? on : off);
  add("Eye of Cthulhu", h.is("killedBoss1") ? on : off);
  if (h.is("crimson")) {
    add("Brain of Cthulhu", h.is("killedBoss2") ? on : off);
  } else {
    add("Eater of Worlds", h.is("killedBoss2") ? on : off);
  }
  add("Goblin Invasion", h.is("killedGoblins") ? on : off);
  add("Skeletron", h.is("killedBoss3") ? on : off);
  add("Queen Bee", h.is("killedQueenBee") ? on : off);
  add("Deerclops", h.is("killedDeerClops") ? on : off);
  add("Wall of Flesh", h.is("hardMode") ? on : off);
  add("Clown", h.is("killedClown") ? on : off);
  add("Pirate Invasion", h.is("killedPirates") ? on : off);
  add("Queen Slime", h.is("killedQueenSlime") ? on : off);
  add("The Destroyer", h.is("killedMechBoss1") ? on : off);
  add("The Twins", h.is("killedMechBoss2") ? on : off);
  add("Skeletron Prime", h.is("killedMechBoss3") ? on : off);
  add("Plantera", h.is("killedPlantBoss") ? on : off);
  add("Golem", h.is("killedGolemBoss") ? on : off);
  add("Mourning Wood", h.is("downedHalloweenTree") ? on : off);
  add("Pumpking", h.is("downedHalloweenKing") ? on : off);
  add("Frost Horde", h.is("killedFrost") ? on : off);
  add("Everscream", h.is("downedChristmasTree") ? on : off);
  add("Santa-NK1", h.is("downedChristmasSantank") ? on : off);
  add("Ice Queen", h.is("downedIceQueen") ? on : off);
  add("Martian Invasion", h.is("downedMartians") ? on : off);
  add("Empress of Light", h.is("killedEmpressOfLight") ? on : off);
  add("Duke Fishron", h.is("downedFishron") ? on : off);
  add("Lunatic Cultist", h.is("downedAncientCultist") ? on : off);
  add("Solar Pillar", h.is("downedSolar") ? on : off);
  add("Vortex Pillar", h.is("downedVortex") ? on : off);
  add("Nebula Pillar", h.is("downedNebula") ? on : off);
  add("Stardust Pillar", h.is("downedStardust") ? on : off);
  add("Moon Lord", h.is("downedMoonlord") ? on : off);
  
}

void InfoWin::show() {
  ImGui::SeparatorText("World Information");
  ImGui::BeginChild("##infolist", ImVec2(400, 200));
  if (ImGui::BeginTable("info", 2)) {
    for (const auto &row : rows) {
      ImGui::TableNextColumn();
      ImGui::Text("%s", row.key);
      ImGui::TableNextColumn();
      ImGui::Text("%s", row.value);
    }
    ImGui::EndTable();
  }
  ImGui::EndChild();
}

void InfoWin::add(const char *key, const char *value) {
  rows.emplace_back(key, value);
}
