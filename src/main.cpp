#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class ReminderNode : public CCNode {
protected:
    float m_timer = 0.f;

    std::vector<std::string> getFrenchMessages() {
        return {
            "Il serait temps d'aller prendre une douche, frero.",
            "Ca fait longtemps que tu joues. Va toucher de l'herbe.",
            "Meme les modos Discord sont choques, bro.",
            "Pourquoi tu ne vas pas voir ta famille un peu ?",
            "Frero, ca va dans ta vie pour jouer autant ??",
            "Stop... arrete de jouer."
        };
    }

    std::vector<std::string> getEnglishMessages() {
        return {
            "It might be time to take a shower, bro.",
            "You have been playing for a long time. Go touch some grass.",
            "Even Discord mods are shocked, bro.",
            "Why don't you go see your family for a bit?",
            "Bro, are you okay to be playing this much??",
            "Stop... stop playing."
        };
    }

    float getReminderInterval() {
        auto mod = Mod::get();

        int amount = mod->getSavedValue<int>("reminder-amount", 30);
        std::string unit = mod->getSavedValue<std::string>("reminder-unit", "minutes");

        if (unit == "hours") {
            return static_cast<float>(amount * 3600);
        }

        return static_cast<float>(amount * 60);
    }

    std::string getRandomMessage() {
        auto mod = Mod::get();
        std::string language = mod->getSavedValue<std::string>("language", "en");

        if (language == "fr") {
            auto msgs = getFrenchMessages();
            int index = rand() % msgs.size();
            return msgs[index];
        }

        auto msgs = getEnglishMessages();
        int index = rand() % msgs.size();
        return msgs[index];
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

        float interval = getReminderInterval();
        if (interval <= 0.f) return;

        if (m_timer >= interval) {
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

class $modify(GoOutsideMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        
        auto mod = Mod::get();

        if (!mod->hasSavedValue("language")) {
            mod->setSavedValue("language", std::string("en"));
        }

        if (!mod->hasSavedValue("reminder-amount")) {
            mod->setSavedValue("reminder-amount", 30);
        }

        if (!mod->hasSavedValue("reminder-unit")) {
            mod->setSavedValue("reminder-unit", std::string("minutes"));
        }

        
        auto reminder = ReminderNode::create();
        this->addChild(reminder);

        return true;
    }
};

$on_mod(Loaded) {
    auto mod = Mod::get();

    log::info("Go outside, bro! loaded");

    
    if (!mod->hasSavedValue("language")) {
        mod->setSavedValue("language", std::string("en"));
    }

    if (!mod->hasSavedValue("reminder-amount")) {
        mod->setSavedValue("reminder-amount", 30);
    }

    if (!mod->hasSavedValue("reminder-unit")) {
        mod->setSavedValue("reminder-unit", std::string("minutes"));
    }
}
