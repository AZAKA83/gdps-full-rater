#include "DevRateStarsPopup.hpp"
#include "managers/SettingsManager.hpp"
#include "utils/Utils.hpp"

#include <Geode/utils/async.hpp>
#include <Geode/utils/web.hpp>
#include <chrono>
#include <cctype>
#include <optional>
#include <string>
#include <vector>

using namespace geode::prelude;

namespace {
struct TierInfo {
    char const* key;
    char const* name;
    char const* icon;
};

// Keep these keys in sync with the PHP add-on's $tiers allow-list.
static std::vector<TierInfo> const& tiers() {
    // Use the original Geometry Dash difficulty sprites already loaded by the game.
    // Frame names are from the game's GJ_GameSheet03 sprite atlas.
    static std::vector<TierInfo> const value = {
        {"auto", "Auto", "difficulty_auto_btn_001.png"},
        {"easy", "Easy", "difficulty_01_btn_001.png"},
        {"normal", "Normal", "difficulty_02_btn_001.png"},
        {"hard", "Hard", "difficulty_03_btn_001.png"},
        {"harder", "Harder", "difficulty_04_btn_001.png"},
        {"insane", "Insane", "difficulty_05_btn_001.png"},
        {"easy-demon", "Easy Demon", "difficulty_06_btn_001.png"},
        {"medium-demon", "Medium Demon", "difficulty_07_btn_001.png"},
        {"hard-demon", "Hard Demon", "difficulty_08_btn_001.png"},
        {"insane-demon", "Insane Demon", "difficulty_09_btn_001.png"},
        {"extreme-demon", "Extreme Demon", "difficulty_10_btn_001.png"},
        {"unrated", "Unrated / Remove", "difficulty_00_btn_001.png"},
    };
    return value;
}

std::string responseBody(web::WebResponse const& response) {
    auto const& bytes = response.data();
    return std::string(bytes.begin(), bytes.end());
}

std::optional<std::string> jsonString(std::string const& json, std::string const& key) {
    auto const marker = fmt::format("\"{}\"", key);
    auto pos = json.find(marker);
    if (pos == std::string::npos) return std::nullopt;
    pos += marker.size();

    // Accept normal JSON whitespace between the property name and colon/value.
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) ++pos;
    if (pos >= json.size() || json[pos++] != ':') return std::nullopt;
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) ++pos;
    if (pos >= json.size() || json[pos++] != '\"') return std::nullopt;

    std::string value;
    bool escaped = false;
    for (; pos < json.size(); ++pos) {
        char const ch = json[pos];
        if (escaped) {
            switch (ch) {
                case '\"': value.push_back('\"'); break;
                case '\\': value.push_back('\\'); break;
                case 'n': value.push_back('\n'); break;
                case 'r': value.push_back('\r'); break;
                case 't': value.push_back('\t'); break;
                default: value.push_back(ch); break;
            }
            escaped = false;
        }
        else if (ch == '\\') {
            escaped = true;
        }
        else if (ch == '\"') {
            return value;
        }
        else {
            value.push_back(ch);
        }
    }
    return std::nullopt;
}

bool jsonBoolTrue(std::string const& json, std::string const& key) {
    auto const marker = fmt::format("\"{}\"", key);
    auto pos = json.find(marker);
    if (pos == std::string::npos) return false;
    pos += marker.size();
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) ++pos;
    if (pos >= json.size() || json[pos++] != ':') return false;
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) ++pos;
    return json.compare(pos, 4, "true") == 0;
}

std::optional<size_t> tierIndexForKey(std::string const& key) {
    auto const& list = tiers();
    for (size_t i = 0; i < list.size(); ++i) {
        if (key == list[i].key) return i;
    }
    return std::nullopt;
}
}

bool DevRateStarsPopup::init(int levelId) {
    if (!Popup::init(390.f, 230.f))
        return false;

    m_levelId = levelId;
    m_selectedTierIndex = tiers().size() - 1; // Unrated until the server says otherwise.

    this->setTitle("GDPS Full Rater", "bigFont.fnt", 1.0f);
    m_title->setPositionY(m_title->getPositionY() - 5.f);
    this->setID("gfr-rate-popup"_spr);

    // M336's original featured/epic/legendary/mythic selector.
    m_featureButton = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_featuredCoin_001.png"),
        this, menu_selector(DevRateStarsPopup::onFeature)
    );
    m_featureButton->setOpacity(DISABLED_OPACITY);
    m_featureButton->setID("feature-button"_spr);
    m_buttonMenu->addChildAtPosition(m_featureButton, Anchor::TopRight, { -10.f, -10.f });

    auto* coinsSprite = CCSprite::createWithSpriteFrameName("secretCoinUI2_001.png");
    coinsSprite->setScale(44.f / 53.75f);
    m_coinsButton = CCMenuItemSpriteExtra::create(
        coinsSprite, this, menu_selector(DevRateStarsPopup::onCoins)
    );
    m_coinsButton->setOpacity(DISABLED_OPACITY);
    m_coinsButton->setID("coins-button"_spr);
    m_buttonMenu->addChildAtPosition(m_coinsButton, Anchor::TopRight, { -10.f, -48.f });

    auto* decreaseStarsButton = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png"),
        this, menu_selector(DevRateStarsPopup::onDecrease)
    );
    decreaseStarsButton->setRotation(-90.f);
    decreaseStarsButton->setID("decrease-stars-button"_spr);
    m_buttonMenu->addChildAtPosition(decreaseStarsButton, Anchor::Left, { 76.f, 18.f });

    m_starsLabel = CCLabelBMFont::create("0", "bigFont.fnt");
    m_starsLabel->setID("stars-label"_spr);
    m_buttonMenu->addChildAtPosition(m_starsLabel, Anchor::Left, { 120.f, 18.f });

    auto* increaseStarsButton = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png"),
        this, menu_selector(DevRateStarsPopup::onIncrease)
    );
    increaseStarsButton->setRotation(90.f);
    increaseStarsButton->setID("increase-stars-button"_spr);
    m_buttonMenu->addChildAtPosition(increaseStarsButton, Anchor::Left, { 164.f, 18.f });

    m_difficultyFace = CCSprite::createWithSpriteFrameName("difficulty_00_btn_001.png");
    m_difficultyFace->setScale(1.1f);
    m_difficultyFace->setID("vanilla-difficulty-face"_spr);
    m_buttonMenu->addChildAtPosition(m_difficultyFace, Anchor::Right, { -75.f, 18.f });

    // Custom difficulty selector: separate from native star rating.
    auto* previous = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png"),
        this, menu_selector(DevRateStarsPopup::onTierPrevious)
    );
    previous->setRotation(-90.f);
    previous->setID("tier-previous"_spr);
    m_buttonMenu->addChildAtPosition(previous, Anchor::Bottom, { -132.f, 77.f });

    auto* next = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png"),
        this, menu_selector(DevRateStarsPopup::onTierNext)
    );
    next->setRotation(90.f);
    next->setID("tier-next"_spr);
    m_buttonMenu->addChildAtPosition(next, Anchor::Bottom, { 132.f, 77.f });

    m_tierIcon = nullptr;
    m_tierLabel = CCLabelBMFont::create("Unrated / Remove", "goldFont.fnt");
    m_tierLabel->setScale(0.55f);
    m_tierLabel->setID("custom-tier-label"_spr);
    m_buttonMenu->addChildAtPosition(m_tierLabel, Anchor::Bottom, { 32.f, 77.f });

    m_tierCounter = CCLabelBMFont::create("12 / 12", "goldFont.fnt");
    m_tierCounter->setScale(0.35f);
    m_tierCounter->setOpacity(180);
    m_buttonMenu->addChildAtPosition(m_tierCounter, Anchor::Bottom, { 32.f, 57.f });
    updateTierVisuals();

    auto* cancel = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Cancel", "goldFont.fnt", "GJ_button_01.png"),
        this, menu_selector(DevRateStarsPopup::onCancel)
    );
    cancel->setID("cancel-button"_spr);
    m_buttonMenu->addChildAtPosition(cancel, Anchor::Bottom, { -112.f, 20.f });

    auto* submitStars = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Stars", "goldFont.fnt", "GJ_button_01.png"),
        this, menu_selector(DevRateStarsPopup::onSubmit)
    );
    submitStars->setID("submit-stars-button"_spr);
    m_buttonMenu->addChildAtPosition(submitStars, Anchor::Bottom, { 0.f, 20.f });

    auto* saveTier = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Save Tier", "goldFont.fnt", "GJ_button_01.png"),
        this, menu_selector(DevRateStarsPopup::onSaveTier)
    );
    saveTier->setID("save-custom-tier-button"_spr);
    m_buttonMenu->addChildAtPosition(saveTier, Anchor::Bottom, { 112.f, 20.f });

    // Optional M336 actions; hidden by default to keep the new selector clear.
    if (SettingsManager::ShowDailyButton) {
        auto* sprite = ButtonSprite::create("Set Daily", 50, true, "goldFont.fnt", "GJ_button_01.png", 40.f, 1.f);
        sprite->setScale(0.48f);
        auto* button = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(DevRateStarsPopup::onDaily));
        m_buttonMenu->addChildAtPosition(button, Anchor::Left, { 28.f, -42.f });
    }
    if (SettingsManager::ShowWeeklyButton) {
        auto* sprite = ButtonSprite::create("Set Week", 50, true, "goldFont.fnt", "GJ_button_01.png", 40.f, 1.f);
        sprite->setScale(0.48f);
        auto* button = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(DevRateStarsPopup::onWeekly));
        m_buttonMenu->addChildAtPosition(button, Anchor::Left, { 28.f, -68.f });
    }
    if (SettingsManager::ShowSendOnlyButton) {
        auto* sprite = ButtonSprite::create("Send Only", 50, true, "goldFont.fnt", "GJ_button_01.png", 40.f, 1.f);
        sprite->setScale(0.48f);
        auto* button = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(DevRateStarsPopup::onSendOnly));
        m_buttonMenu->addChildAtPosition(button, Anchor::Right, { -28.f, -68.f });
    }

    loadSavedTier();
    return true;
}

void DevRateStarsPopup::onFeature(CCObject*) {
    auto* sprite = static_cast<CCSprite*>(m_featureButton->getNormalImage());
    auto* cache = CCSpriteFrameCache::sharedSpriteFrameCache();
    m_featureState = static_cast<FeatureState>((static_cast<int>(m_featureState) + 1) % 5);
    char const* frame = "GJ_featuredCoin_001.png";
    switch (m_featureState) {
        case FeatureState::Epic: frame = "GJ_epicCoin_001.png"; break;
        case FeatureState::Legendary: frame = "GJ_epicCoin2_001.png"; break;
        case FeatureState::Mythic: frame = "GJ_epicCoin3_001.png"; break;
        default: break;
    }
    if (auto* selectedFrame = cache->spriteFrameByName(frame)) sprite->setDisplayFrame(selectedFrame);
    m_featureButton->setOpacity(m_featureState == FeatureState::None ? DISABLED_OPACITY : ENABLED_OPACITY);
}

void DevRateStarsPopup::onCoins(CCObject*) {
    m_coinsEnabled = !m_coinsEnabled;
    m_coinsButton->setOpacity(m_coinsEnabled ? ENABLED_OPACITY : DISABLED_OPACITY);
}

void DevRateStarsPopup::onDecrease(CCObject*) {
    if (m_selectedStars <= 0) return;
    --m_selectedStars;
    updateDifficultyVisuals(m_selectedStars);
}

void DevRateStarsPopup::onIncrease(CCObject*) {
    if (m_selectedStars >= 10) return;
    ++m_selectedStars;
    updateDifficultyVisuals(m_selectedStars);
}

void DevRateStarsPopup::onTierPrevious(CCObject*) {
    auto const count = tiers().size();
    m_selectedTierIndex = (m_selectedTierIndex + count - 1) % count;
    updateTierVisuals();
}

void DevRateStarsPopup::onTierNext(CCObject*) {
    m_selectedTierIndex = (m_selectedTierIndex + 1) % tiers().size();
    updateTierVisuals();
}

void DevRateStarsPopup::updateTierVisuals() {
    if (m_selectedTierIndex >= tiers().size()) m_selectedTierIndex = tiers().size() - 1;
    auto const& tier = tiers()[m_selectedTierIndex];
    if (m_tierIcon) {
        m_tierIcon->removeFromParentAndCleanup(true);
        m_tierIcon = nullptr;
    }
    auto* frameCache = CCSpriteFrameCache::sharedSpriteFrameCache();
    auto* frame = frameCache->spriteFrameByName(tier.icon);
    if (frame) {
        m_tierIcon = CCSprite::createWithSpriteFrame(frame);
    } else {
        // Safe visual fallback if a texture pack or game version lacks a frame.
        log::warn("Could not find original difficulty frame: {}", tier.icon);
        m_tierIcon = CCSprite::createWithSpriteFrameName("difficulty_00_btn_001.png");
    }
    if (m_tierIcon) {
        m_tierIcon->setScale(1.0f);
        m_tierIcon->setID("original-difficulty-icon"_spr);
        m_buttonMenu->addChildAtPosition(m_tierIcon, Anchor::Bottom, { -55.f, 77.f });
    }
    m_tierLabel->setString(tier.name);
    m_tierCounter->setString(fmt::format("{} / {}", m_selectedTierIndex + 1, tiers().size()).c_str());
}

void DevRateStarsPopup::setTierByKey(std::string const& key) {
    if (auto index = tierIndexForKey(key)) {
        m_selectedTierIndex = *index;
        updateTierVisuals();
    }
}

void DevRateStarsPopup::loadSavedTier() {
    auto base = Utils::getApiBase();
    if (base.empty() || !base.starts_with("https://")) return;

    auto url = base + "gdpsFullRaterGet.php?levelID=" + std::to_string(m_levelId);
    Ref<DevRateStarsPopup> self = this;
    async::spawn(
        web::WebRequest().timeout(std::chrono::seconds(15)).get(url),
        [self](web::WebResponse response) {
            if (!self || !response.ok()) {
                log::warn("Could not load custom tier (HTTP {})", response.code());
                return;
            }
            auto body = responseBody(response);
            if (body.find("\"rating\":null") != std::string::npos) {
                self->setTierByKey("unrated");
                return;
            }
            if (auto key = jsonString(body, "key")) self->setTierByKey(*key);
        }
    );
}

void DevRateStarsPopup::saveSelectedTier() {
    auto base = Utils::getApiBase();
    if (base.empty()) {
        FLAlertLayer::create("GDPS API URL missing", "Open GDPS Full Rater settings and set <cy>Server URL</c> to your GDPS <cy>/database/</c> folder URL.", "OK")->show();
        return;
    }
    if (!base.starts_with("https://")) {
        FLAlertLayer::create("HTTPS required", "For account safety, the mod only sends your GDPS GJP2 token over an <cy>HTTPS</c> connection.", "OK")->show();
        return;
    }

    auto* account = GJAccountManager::sharedState();
    if (!account || account->m_accountID <= 0 || account->m_GJP2.empty()) {
        FLAlertLayer::create("GDPS login required", "Log in to your GDPS account in Geometry Dash first.", "OK")->show();
        return;
    }

    auto const& tier = tiers()[m_selectedTierIndex];
    web::MultipartForm form;
    form.param("accountID", std::to_string(account->m_accountID));
    form.param("gjp2", std::string(account->m_GJP2.c_str()));
    form.param("levelID", std::to_string(m_levelId));
    form.param("tier", std::string(tier.key));

    auto url = base + "gdpsFullRaterSet.php";
    auto selectedName = std::string(tier.name);
    Ref<DevRateStarsPopup> self = this;
    async::spawn(
        web::WebRequest().timeout(std::chrono::seconds(20)).bodyMultipart(form).post(url),
        [self, selectedName, levelID = m_levelId](web::WebResponse response) {
            if (!self) return;
            auto body = responseBody(response);
            if (response.ok() && jsonBoolTrue(body, "ok")) {
                auto const code = jsonString(body, "code").value_or("SAVED");
                auto const message = code == "CUSTOM_TIER_REMOVED"
                    ? fmt::format("Removed the custom difficulty from level <cj>{}</c>.", levelID)
                    : fmt::format("Saved <cy>{}</c> for level <cj>{}</c>.", selectedName, levelID);
                FLAlertLayer::create("GDPS Full Rater", message, "OK")->show();
                return;
            }
            auto code = jsonString(body, "code").value_or("UNKNOWN_ERROR");
            std::string explanation = code;
            if (code == "AUTH_REQUIRED" || code == "INVALID_GJP2") explanation = "Your GDPS login could not be verified. Log in again and retry.";
            else if (code == "AUTH_RATE_LIMITED") explanation = "Too many failed login checks came from this network. Wait before trying again.";
            else if (code == "ACCOUNT_INACTIVE") explanation = "This GDPS account is not activated.";
            else if (code == "MISSING_COMMAND_RATE_PERMISSION") explanation = "Your GDPS account does not have commandRate permission.";
            else if (code == "LEVEL_NOT_FOUND") explanation = "This level ID was not found on the configured GDPS.";
            else if (code == "UNKNOWN_TIER") explanation = "The server does not recognize this custom tier. Update its PHP allow-list.";
            FLAlertLayer::create("Rating was not saved", explanation, "OK")->show();
            log::warn("GDPS Full Rater save failed (HTTP {}, code {})", response.code(), code);
        }
    );
}

void DevRateStarsPopup::onSaveTier(CCObject*) {
    saveSelectedTier();
}

void DevRateStarsPopup::onDaily(CCObject*) {
    GameLevelManager::sharedState()->uploadComment("!daily", CommentType::Level, m_levelId, 0);
    this->onClose(nullptr);
}

void DevRateStarsPopup::onWeekly(CCObject*) {
    GameLevelManager::sharedState()->uploadComment("!weekly", CommentType::Level, m_levelId, 0);
    this->onClose(nullptr);
}

void DevRateStarsPopup::onSendOnly(CCObject*) {
    if (m_selectedStars > 0) {
        GameLevelManager::sharedState()->uploadComment(
            fmt::format("!send {} {} {}", Utils::getLevelDifficulty(m_selectedStars), m_selectedStars, static_cast<int>(m_featureState)),
            CommentType::Level, m_levelId, 0
        );
    } else {
        GameLevelManager::sharedState()->uploadComment("!unsend", CommentType::Level, m_levelId, 0);
    }
    this->onClose(nullptr);
}

void DevRateStarsPopup::onCancel(CCObject*) {
    this->onClose(nullptr);
}

void DevRateStarsPopup::onSubmit(CCObject*) {
    if (m_selectedStars > 0) {
        GameLevelManager::sharedState()->uploadComment(
            fmt::format("!rate {} {} {} {}", Utils::getLevelDifficulty(m_selectedStars), m_selectedStars,
                m_coinsEnabled ? 1 : 0, static_cast<int>(m_featureState)),
            CommentType::Level, m_levelId, 0
        );
    } else {
        GameLevelManager::sharedState()->uploadComment("!unrate", CommentType::Level, m_levelId, 0);
    }
    this->onClose(nullptr);
}

void DevRateStarsPopup::updateDifficultyVisuals(int stars) {
    static constexpr std::array difficultyFrames = {
        "difficulty_00_btn_001.png", "difficulty_auto_btn_001.png", "difficulty_01_btn_001.png",
        "difficulty_02_btn_001.png", "difficulty_03_btn_001.png", "difficulty_03_btn_001.png",
        "difficulty_04_btn_001.png", "difficulty_04_btn_001.png", "difficulty_05_btn_001.png",
        "difficulty_05_btn_001.png", "difficulty_06_btn_001.png"
    };
    auto frame = (stars >= 0 && stars < static_cast<int>(difficultyFrames.size()))
        ? difficultyFrames[stars] : "difficulty_00_btn_001.png";
    if (auto* selectedFrame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(frame)) {
        m_difficultyFace->setDisplayFrame(selectedFrame);
    }
    m_starsLabel->setString(std::to_string(stars).c_str());
}

DevRateStarsPopup* DevRateStarsPopup::create(int levelId) {
    auto* ret = new DevRateStarsPopup();
    if (ret->init(levelId)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}
