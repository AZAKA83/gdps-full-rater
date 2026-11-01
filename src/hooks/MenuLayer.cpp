#include <Geode/Geode.hpp>
#include "../utils/Utils.hpp"

using namespace geode::prelude;

#include <Geode/modify/MenuLayer.hpp>
class $modify(MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        static bool shown = false;
        if (!shown && !Utils::isOnGdps()) {
            auto* alert = FLAlertLayer::create(
                "GDPS Full Rater not configured",
                "This build uses the Cvolton-compatible PHP add-on. Set <cy>Server URL</c> to the trusted HTTPS folder that contains <cy>gdpsFullRaterGet.php</c> and <cy>gdpsFullRaterSet.php</c> (usually <cy>/database/</c>). A Supabase project URL by itself is not compatible with this build.",
                "OK"
            );
            alert->m_scene = this;
            alert->show();
            shown = true;
        }
        return true;
    }
};
