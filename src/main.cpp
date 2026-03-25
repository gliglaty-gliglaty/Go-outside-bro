#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class ReminderNode : public CCNode {
protected:
    float m_timer = 0.f;

    std::vector<std::string> getFrenchMessages() {
        return {
            "Il serait temps d'aller se doucher, frero.",
            "Ca fait un moment que tu joues. Va toucher de l'herbe.",
            "Meme les modos Discord sont choques de ton temps de jeu, bro.",
            "Pourquoi tu ne vas pas voir ta famille au lieu de jouer ?",
            "Frero, ca va dans ta vie pour jouer autant ??",
            "Stop... arrete de jouer."
        };
    }

    std::vector<std::string> getEnglishMessages() {
        return {
            "It might be time to take a shower, bro.",
            "You've been playing for a while. Go touch some grass.",
            "Even Discord mods are shocked by your playtime, bro.",
            "Why don't you go see your family instead of playing?",
            "Bro, are you okay to be playing this much??",
            "Stop... stop playing."
        };
    }

    float getReminderSeconds() {
        auto mod = Mod::get();

        int amount = static_cast<int>(mod->getSettingValue<int64_t>("reminder-amount"));
        std::string unit = mod->getSettingValue<std::string>("reminder-unit");

        if (amount < 1) amount = 1;

        if (unit == "hours") {
            return static_cast<float>(amount * 3600);
        }

        return static_cast<float>(amount * 60);
    }

    std::string getRandomMessage() {
        auto mod = Mod::get();
        std::string language = mod->getSettingValue<std::string>("language");

        if (language == "fr") {
            auto msgs = getFrenchMessages();
            return msgs[rand() % msgs.size()];
        }

        auto msgs = getEnglishMessages();
        return msgs[rand() % msgs.size()];
    }

public:
    static ReminderNode* create() {
        auto ret = new ReminderNode();
        if (ret && ret->init()) {
            ret->autorelease();
            ret->schedule(schedule_selector(ReminderNode::tick), 1.0f);
            return ret;
        }

        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    void tick(float dt) {
        m_timer += dt;

        float needed = getReminderSeconds();
        if (needed <= 0.f) return;

        if (m_timer >= needed) {
            m_timer = 0.f;

            auto message = getRandomMessage();

            Notification::create(
                message,
                NotificationIcon::Info,
                4.0f
            )->show();

            FMODAudioEngine::sharedEngine()->playEffect("achievement_01.ogg");
        }
    }
};

class $modify(GoOutsideBroMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto reminder = ReminderNode::create();
        this->addChild(reminder);

        return true;
    }
};

$on_mod(Loaded) {
    log::info("Go outside, bro! loaded");
}
