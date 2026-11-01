#include "Utils.hpp"
#include <algorithm>
#include <cctype>

using namespace geode::prelude;

namespace Utils {
    std::string getApiBase() {
        auto base = Mod::get()->getSettingValue<std::string>("server-url");
        auto first = base.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return {};
        auto last = base.find_last_not_of(" \t\r\n");
        base = base.substr(first, last - first + 1);
        while (!base.empty() && base.back() == '/') base.pop_back();
        if (base.empty()) return {};
        return base + "/";
    }

    bool isOnGdps() {
        auto base = getApiBase();
        if (!base.starts_with("https://"))
            return false;

        // This source build calls the Cvolton-compatible PHP endpoints directly.
        // A Supabase project root is not a compatible PHP API base.
        if (base.find(".supabase.co") != std::string::npos)
            return false;

        return true;
    }
}
