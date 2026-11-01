#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>

#include "../DevRateStarsPopup.hpp"
#include "../managers/SessionManager.hpp"
#include "../utils/Utils.hpp"

using namespace geode::prelude;

class $modify(GFRLevelInfoLayer, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge))
            return false;

        // The custom server URL is configured by the user in this mod's settings.
        // Do not show the moderation button when no HTTPS API base is configured.
        if (!level || level->m_levelID <= 0 || !Utils::isOnGdps())
            return true;

        if (auto* leftMenu = static_cast<CCMenu*>(this->getChildByID("left-side-menu"))) {
            auto* rateButton = CCMenuItemSpriteExtra::create(
                CCSprite::createWithSpriteFrameName("GJ_starBtnMod_001.png"),
                this,
                menu_selector(GFRLevelInfoLayer::onGFRRate)
            );
            rateButton->setColor({ 74, 242, 250 });
            rateButton->setID("gfr-rate-button"_spr);
            leftMenu->addChild(rateButton);
            leftMenu->updateLayout();
        }

        return true;
    }

    void onGFRRate(CCObject*) {
        auto const levelID = static_cast<int>(m_level->m_levelID);

        // This warning follows M336's RateButton behavior: it explains that
        // the account needs server-side commandRate permission before submit.
        if (!SessionManager::ShownRateButtonWarning) {
            createQuickPopup(
                "Rate Button Warning",
                "This tool requires a GDPS account with <cy>commandRate permission</c>.\n"
                "The server checks your GDPS login before saving a custom difficulty.\n\n"
                "Only continue if you trust the configured HTTPS server.",
                "Cancel", "Continue",
                [levelID](auto, bool confirmed) {
                    if (!confirmed)
                        return;

                    Mod::get()->setSavedValue("shown-rate-button-warning", true);
                    SessionManager::ShownRateButtonWarning = true;
                    if (auto* popup = DevRateStarsPopup::create(levelID))
                        popup->show();
                }
            );
            return;
        }

        if (auto* popup = DevRateStarsPopup::create(levelID))
            popup->show();
    }
};
