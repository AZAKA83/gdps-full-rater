#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>

using namespace geode::prelude;

// This starter adds a button on a level's information page. It does NOT submit
// ratings yet: the GDPS-specific comment/upload API and custom-rating endpoint
// must be connected for the selected tier to be saved server-side.
class $modify(GDPSFullRaterLevelInfoLayer, LevelInfoLayer) {
    struct Fields {
        int levelID = 0;
    };

    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;
        m_fields->levelID = level ? level->m_levelID : 0;

        auto menu = CCMenu::create();
        if (!menu) return true;
        menu->setID("gdps-full-rater-menu");
        menu->setPosition({0.f, 0.f});
        this->addChild(menu, 100);

        auto sprite = ButtonSprite::create("Rate");
        auto button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(GDPSFullRaterLevelInfoLayer::onOpenFullRater)
        );
        if (button) {
            button->setID("gdps-full-rater-open");
            // Bottom-left placement; adjust if another mod uses this area.
            button->setScale(0.65f);
            button->setPosition({42.f, 31.f});
            menu->addChild(button);
            menu->updateLayout();
        }
        return true;
    }

    void onOpenFullRater(CCObject*) {
        std::string levelID = std::to_string(m_fields->levelID);
        std::string body =
            "Level ID: " + levelID +
            "\n\nCustom tiers:\n"
            "Free, Peaceful, Simple, Casual, Mild, Moderate\n"
            "Tricky, Tough, Wild, Cruel, Crazy, Bizarre, Brutal\n"
            "Supreme, Ultimate, Legendary, Mythical, Infinite, Grandpa\n"
            "\nSaving custom ratings is not connected yet.";
        FLAlertLayer::create("GDPS Rater", body, "Close")->show();
    }
};
