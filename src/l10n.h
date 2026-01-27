/** @copyright 2025 Sean Kasun */

#pragma once

/*
This is terrafirma's localization class.
It parses the language files from terraria.exe to translate item names
*/

#include "json.h"
#include <string>
#include <set>

class L10n {
  public:
    void load(std::string exe);
    std::string xlateItem(const std::string &key) const;
    std::string xlatePrefix(const std::string &key) const;
    std::string xlateNPC(const std::string &key) const;
    std::vector<std::string> getLanguages() const;
    void setLanguage(std::string lang);
    std::string selectedLanguage() const;

  private:
    std::unordered_map<std::string, std::shared_ptr<JSONData>> items;
    std::unordered_map<std::string, std::shared_ptr<JSONData>> prefixes;
    std::unordered_map<std::string, std::shared_ptr<JSONData>> npcs;
    std::set<std::string> languages;
    std::string currentLanguage = "en-US";
};
