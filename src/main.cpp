#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

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

        bool testMode = false;
        if (mod->hasSetting("test-mode")) {
            testMode = mod->getSettingValue<bool>("test-mode");
        }

        if (testMode) {
            return 5.f;
        }

        int amount = 30;
        bool useHours = false;

        if (mod->hasSetting("reminder-amount")) {
            amount = static_cast<int>(mod->getSettingValue<int64_t>("reminder-amount"));
        }

        if (mod->hasSetting("reminder-hours")) {
            useHours = mod->getSettingValue<bool>("reminder-hours");
        }

        if (amount < 1) amount = 1;

        if (useHours) {
            return static_cast<float>(amount * 3600);
        }

        return static_cast<float>(amount * 60);
    }

    std::string getRandomMessage() {
        auto mod = Mod::get();
        bool french = false;

        if (mod->hasSetting("language-french")) {
            french = mod->getSettingValue<bool>("language-french");
        }

        if (french) {
            auto msgs = getFrenchMessages();
            return msgs[rand() % msgs.size()];
        }

        auto msgs = getEnglishMessages();
        return msgs[rand() % msgs.size()];
    }

    void showReminder() {
        auto message = getRandomMessage();

        Notification::create(
            "Go outside, bro!\n" + message,
            NotificationIcon::Info,
            4.0f
        )->show();

        FMODAudioEngine::sharedEngine()->playEffect("achievement_01.ogg");
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
            showReminder();
        }
    }
};

static void attachReminderNode(CCNode* parent) {
    if (!parent) return;

    if (parent->getChildByID("gooutside-reminder-node")) return;

    auto reminder = ReminderNode::create();
    reminder->setID("gooutside-reminder-node");
    parent->addChild(reminder, 9999);
}

class $modify(GoOutsideBroMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        attachReminderNode(this);
        return true;
    }
};

class $modify(GoOutsideBroPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }

        attachReminderNode(this);
        return true;
    }
};

$on_mod(Loaded) {
    log::info("Go outside, bro! loaded");
}
