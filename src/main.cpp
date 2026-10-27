#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/utils/web.hpp>
#include <km7dev.server_api/include/ServerAPIEvents.hpp>

#include <algorithm>
#include <cctype>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

using namespace geode::prelude;

namespace {
    struct Tier {
        char const* key;
        char const* name;
    };

    // These are the extra custom tiers supported by server-addon/cvolton.
    // The last entry removes the custom tier instead of setting a rating.
    constexpr Tier kTiers[] = {
        {"auto", "Auto"},
        {"easy", "Easy"},
        {"normal", "Normal"},
        {"hard", "Hard"},
        {"harder", "Harder"},
        {"insane", "Insane"},
        {"easy-demon", "Easy Demon"},
        {"medium-demon", "Medium Demon"},
        {"hard-demon", "Hard Demon"},
        {"insane-demon", "Insane Demon"},
        {"extreme-demon", "Extreme Demon"},
        {"free-demon", "Free Demon"},
        {"peaceful-demon", "Peaceful Demon"},
        {"simple-demon", "Simple Demon"},
        {"casual-demon", "Casual Demon"},
        {"mild-demon", "Mild Demon"},
        {"moderate-demon", "Moderate Demon"},
        {"tricky-demon", "Tricky Demon"},
        {"tough-demon", "Tough Demon"},
        {"wild-demon", "Wild Demon"},
        {"cruel-demon", "Cruel Demon"},
        {"crazy-demon", "Crazy Demon"},
        {"bizarre-demon", "Bizarre Demon"},
        {"brutal-demon", "Brutal Demon"},
        {"supreme-demon", "Supreme Demon"},
        {"ultimate-demon", "Ultimate Demon"},
        {"legendary-demon", "Legendary Demon"},
        {"mythical-demon", "Mythical Demon"},
        {"infinite-demon", "Infinite Demon"},
        {"grandpa-demon", "Grandpa Demon"},
        {"unrated", "Remove custom rating"},
    };

    std::string urlEncode(std::string const& value) {
        static char const* hex = "0123456789ABCDEF";
        std::string out;
        out.reserve(value.size() * 3);
        for (unsigned char c : value) {
            if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                out.push_back(static_cast<char>(c));
            } else {
                out.push_back('%');
                out.push_back(hex[(c >> 4) & 0x0F]);
                out.push_back(hex[c & 0x0F]);
            }
        }
        return out;
    }

    std::string jsonStringField(std::string const& json, std::string const& field) {
        const auto needle = std::string("\"") + field + "\"";
        auto pos = json.find(needle);
        if (pos == std::string::npos) return {};
        pos = json.find(':', pos + needle.size());
        if (pos == std::string::npos) return {};
        pos = json.find('"', pos + 1);
        if (pos == std::string::npos) return {};
        auto end = json.find('"', pos + 1);
        if (end == std::string::npos) return {};
        return json.substr(pos + 1, end - pos - 1);
    }

    // Server API (provided by GDPS Switcher) knows which server is currently active.
    // Its URL is the server's API base; never ask users to type a URL into this mod.
        std::string activeServerURL() {
        auto url = ServerAPIEvents::getCurrentServer().url;
        const auto first = url.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return {};
        const auto last = url.find_last_not_of(" \t\r\n");
        url = url.substr(first, last - first + 1);
        // Some server profiles may contain only host/path; normalize them for detection.
        if (url.find("://") == std::string::npos) {
            url = "https://" + url;
        }
        while (!url.empty() && url.back() == '/') url.pop_back();
        return url;
    }

    std::string lowerCopy(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    std::string hostFromURL(std::string const& url) {
        auto scheme = url.find("://");
        if (scheme == std::string::npos) return {};
        auto start = scheme + 3;
        auto end = url.find_first_of("/:?#", start);
        auto host = url.substr(start, end == std::string::npos ? std::string::npos : end - start);
        return lowerCopy(host);
    }

    bool isGDPSURL(std::string const& url) {
        auto host = hostFromURL(url);
        if (host.empty()) return false;
        // Geometry Dash's official server runs on boomlings.com. A different active
        // server selected through GDPS Switcher is treated as a GDPS.
        if (host == "boomlings.com" || host.ends_with(".boomlings.com")) return false;
        return true;
    }

    bool isSecureBaseURL(std::string const& url) {
        auto lowered = lowerCopy(url);
        return lowered.rfind("https://", 0) == 0;
    }

    // Cvolton-based GDPS installations commonly store their API in /database.
    // Server API may already return that path; add it only when missing.
    std::string activeAPIBaseURL() {
        auto base = activeServerURL();
        while (!base.empty() && base.back() == '/') base.pop_back();
        if (base.empty()) return {};
        auto lowered = lowerCopy(base);
        constexpr char path[] = "/database";
        if (lowered.size() >= sizeof(path) - 1 &&
            lowered.compare(lowered.size() - (sizeof(path) - 1), sizeof(path) - 1, path) == 0) {
            return base;
        }
        return base + path;
    }
}

class $modify(GDPSFullRaterLevelInfoLayer, LevelInfoLayer) {
    struct Fields {
        int levelID = 0;
        size_t selectedTier = 0;
        std::string currentTierName = "No custom rating saved";
        async::TaskHolder<web::WebResponse> request;
    };

    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;
        m_fields->levelID = level ? level->m_levelID : 0;

        // Do not show the rating tool on the official Geometry Dash server.
        // GDPS Switcher registers the selected server with Server API.
        auto detectedURL = activeServerURL();
        auto detectedHost = hostFromURL(detectedURL);
        if (detectedHost == "boomlings.com" || detectedHost.ends_with(".boomlings.com")) return true;

        auto menu = CCMenu::create();
        if (!menu) return true;
        menu->setID("gdps-full-rater-menu");
        menu->setPosition({0.f, 0.f});
        this->addChild(menu, 100);

        auto sprite = ButtonSprite::create("RATE");
        auto button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(GDPSFullRaterLevelInfoLayer::onOpenFullRater)
        );
        if (button) {
            button->setID("gdps-full-rater-open");
            button->setScale(0.62f);
            button->setPosition({42.f, 31.f});
            menu->addChild(button);
            menu->updateLayout();
        }
        return true;
    }

    void onOpenFullRater(CCObject*) {
        if (m_fields->levelID <= 0) {
            FLAlertLayer::create("GDPS Rater", "This level does not have a valid online level ID.", "OK")->show();
            return;
        }

        auto base = activeServerURL();
        if (!isGDPSURL(base)) {
            FLAlertLayer::create("GDPS Rater", "GDPS Switcher returned no usable server URL. Select your Glowhost server in GDPS Switcher and reopen this level.", "OK")->show();
            return;
        }
        if (!isSecureBaseURL(base)) {
            FLAlertLayer::create("GDPS Rater", "This server URL is not HTTPS. Rating requests contain your GDPS account token, so they were blocked for safety.", "OK")->show();
            return;
        }

        auto apiBase = activeAPIBaseURL();
        if (apiBase.empty()) {
            FLAlertLayer::create("GDPS Rater", "Server API did not provide an active server URL.", "OK")->show();
            return;
        }
        auto req = web::WebRequest();
        req.param("levelID", std::to_string(m_fields->levelID));
        m_fields->request.spawn(
            "Read GDPS custom rating",
            req.get(apiBase + "/gdpsFullRaterGet.php"),
            [this](web::WebResponse response) {
                auto text = response.string().unwrapOr("");
                m_fields->currentTierName = "No custom rating saved";
                auto key = jsonStringField(text, "key");
                if (!key.empty()) {
                    for (size_t i = 0; i < std::size(kTiers); ++i) {
                        if (key == kTiers[i].key) {
                            m_fields->selectedTier = i;
                            m_fields->currentTierName = kTiers[i].name;
                            break;
                        }
                    }
                } else {
                    m_fields->selectedTier = 0;
                }
                if (!response.ok()) {
                    m_fields->currentTierName = "Could not read current rating; you can still try to submit";
                }
                showTierPicker();
            }
        );
    }

    void showTierPicker() {
        const auto index = m_fields->selectedTier % std::size(kTiers);
        const auto& tier = kTiers[index];
        std::string body = "Level ID: " + std::to_string(m_fields->levelID) +
            "\nCurrently saved: " + m_fields->currentTierName +
            "\n\nSelected tier: " + std::string(tier.name) +
            "\n\nTap Next tier to browse, or Save rating to store this custom tier on the GDPS.\n"
            "Your GDPS must have the server add-on installed, and your account needs commandRate permission.";

        createQuickPopup(
            "GDPS Rater",
            body,
            "Next tier",
            "Save rating",
            [this](FLAlertLayer*, bool save) {
                if (save) {
                    submitSelectedTier();
                } else {
                    m_fields->selectedTier = (m_fields->selectedTier + 1) % std::size(kTiers);
                    showTierPicker();
                }
            }
        );
    }

    void submitSelectedTier() {
        auto base = activeServerURL();
        if (!isGDPSURL(base)) {
            FLAlertLayer::create("GDPS Rater", "GDPS Switcher returned no usable server URL. Select your Glowhost server in GDPS Switcher and reopen this level.", "OK")->show();
            return;
        }
        if (!isSecureBaseURL(base)) {
            FLAlertLayer::create("GDPS Rater", "This server URL is not HTTPS. Rating requests contain your GDPS account token, so they were blocked for safety.", "OK")->show();
            return;
        }

        auto account = GJAccountManager::sharedState();
        if (!account || account->m_accountID <= 0 || account->m_GJP2.empty()) {
            FLAlertLayer::create(
                "GDPS Rater",
                "You must be logged into a GDPS account in Geometry Dash before rating. The account token is used only for this HTTPS request.",
                "OK"
            )->show();
            return;
        }

        auto apiBase = activeAPIBaseURL();
        const auto& tier = kTiers[m_fields->selectedTier % std::size(kTiers)];
        const std::string accountID = std::to_string(account->m_accountID);
        const std::string gjp2 = account->m_GJP2.c_str();
        const std::string body =
            "accountID=" + urlEncode(accountID) +
            "&gjp2=" + urlEncode(gjp2) +
            "&levelID=" + urlEncode(std::to_string(m_fields->levelID)) +
            "&tier=" + urlEncode(tier.key);

        auto req = web::WebRequest();
        req.header("Content-Type", "application/x-www-form-urlencoded");
        req.bodyString(body);
        m_fields->request.spawn(
            "Save GDPS custom rating",
            req.post(apiBase + "/gdpsFullRaterSet.php"),
            [this, selectedName = std::string(tier.name)](web::WebResponse response) {
                auto text = response.string().unwrapOr("");
                const auto code = jsonStringField(text, "code");
                if (response.ok() && text.find("\"ok\":true") != std::string::npos) {
                    if (code == "CUSTOM_TIER_REMOVED") {
                        m_fields->currentTierName = "No custom rating saved";
                        FLAlertLayer::create("GDPS Rater", "Custom rating removed from the GDPS database.", "OK")->show();
                    } else {
                        m_fields->currentTierName = selectedName;
                        FLAlertLayer::create("GDPS Rater", "Saved " + selectedName + " to your GDPS database.", "OK")->show();
                    }
                    return;
                }

                std::string message = "The GDPS rejected the rating (HTTP " + std::to_string(response.code()) + ").";
                if (!code.empty()) message += "\nServer response: " + code;
                else if (text.empty()) message += "\nNo response received. Check the server URL and PHP add-on installation.";
                else message += "\nResponse: " + text.substr(0, 180);
                if (code == "MISSING_COMMAND_RATE_PERMISSION") {
                    message += "\nGrant your account the Cvolton commandRate permission.";
                }
                FLAlertLayer::create("GDPS Rater", message, "OK")->show();
            }
        );
    }
};
