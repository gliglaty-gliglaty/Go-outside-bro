#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/CCScheduler.hpp>

using namespace geode::prelude;

static float g_reminderTimer = 0.f;
static float g_testTimer = 0.f;

static std::string formatTime(double totalSeconds) {
    int seconds = static_cast<int>(totalSeconds);

    int days = seconds / 86400;
    seconds %= 86400;

    int hours = seconds / 3600;
    seconds %= 3600;

    int minutes = seconds / 60;
    seconds %= 60;

    std::string result;

    if (days > 0) result += std::to_string(days) + "d ";
    if (hours > 0 || days > 0) result += std::to_string(hours) + "h ";
    if (minutes > 0 || hours > 0 || days > 0) result += std::to_string(minutes) + "m ";
    result += std::to_string(seconds) + "s";

    return result;
}

class TopReminderNotification : public CCNode {
public:
    static TopReminderNotification* create(std::string const& text) {
        auto ret = new TopReminderNotification();
        if (ret && ret->init()) {
            ret->autorelease();
            ret->setup(text);
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    void setup(std::string const& text) {
        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto bg = CCScale9Sprite::create("square02b_001.png");
        if (bg) {
            bg->setContentSize(CCSize(340.f, 42.f));
            bg->setOpacity(150);
            bg->setColor(ccc3(0, 0, 0));
            this->addChild(bg);
        }

        auto label = CCLabelBMFont::create(
            text.c_str(),
            "chatFont.fnt",
            300.f,
            kCCTextAlignmentCenter
        );

        if (label) {
            label->setScale(0.72f);
            label->setPosition(CCPointZero);
            this->addChild(label);
        }

        this->setPosition(ccp(winSize.width / 2, winSize.height + 40.f));

        auto moveIn = CCEaseSineOut::create(
            CCMoveTo::create(0.25f, ccp(winSize.width / 2, winSize.height - 25.f))
        );
        auto delay = CCDelayTime::create(4.5f);
        auto moveOut = CCEaseSineIn::create(
            CCMoveTo::create(0.25f, ccp(winSize.width / 2, winSize.height + 40.f))
        );
        auto cleanup = CCCallFunc::create(this, callfunc_selector(TopReminderNotification::removeSelf));

        this->runAction(CCSequence::create(moveIn, delay, moveOut, cleanup, nullptr));
    }

    void removeSelf() {
        this->removeFromParentAndCleanup(true);
    }
};

static bool getNotificationsEnabled() {
    auto mod = Mod::get();
    return mod->hasSetting("enable-notifications")
        ? mod->getSettingValue<bool>("enable-notifications")
        : true;
}

static bool getTestMode() {
    auto mod = Mod::get();
    return mod->hasSetting("test-mode")
        ? mod->getSettingValue<bool>("test-mode")
        : false;
}

static bool getFrenchEnabled() {
    auto mod = Mod::get();
    return mod->hasSetting("language-french")
        ? mod->getSettingValue<bool>("language-french")
        : false;
}

static bool getUseHours() {
    auto mod = Mod::get();
    return mod->hasSetting("reminder-hours")
        ? mod->getSettingValue<bool>("reminder-hours")
        : false;
}

static int getReminderAmount() {
    auto mod = Mod::get();
    if (mod->hasSetting("reminder-amount")) {
        int amount = static_cast<int>(mod->getSettingValue<int64_t>("reminder-amount"));
        return std::max(1, amount);
    }
    return 30;
}

static float getReminderSeconds() {
    int amount = getReminderAmount();
    return getUseHours() ? static_cast<float>(amount * 3600) : static_cast<float>(amount * 60);
}

static std::vector<std::string> getFrenchMessages() {
    return {
        "Il serait temps d'aller se doucher, frero.",
        "Ca fait un moment que tu joues. Va toucher de l'herbe.",
        "Meme les modos Discord sont choques de ton temps de jeu, bro.",
        "Pourquoi tu ne vas pas voir ta famille au lieu de jouer ?",
        "Frero, ca va dans ta vie pour jouer autant ??",
        "Stop... arrete de jouer.",
        "Va prendre l'air deux minutes.",
        "Tes yeux meritent une pause, bro.",
        "Tu joues depuis un bon moment deja.",
        "Petit rappel : le soleil existe encore."
    };
}

static std::vector<std::string> getEnglishMessages() {
    return {
        "It might be time to take a shower, bro.",
        "You've been playing for a while. Go touch some grass.",
        "Even Discord mods are shocked by your playtime, bro.",
        "Why don't you go see your family instead of playing?",
        "Bro, are you okay to be playing this much??",
        "Stop... stop playing.",
        "Go get some fresh air for a minute.",
        "Your eyes deserve a break, bro.",
        "You've been playing for quite a while already.",
        "Reminder: the sun still exists."
    };
}

static std::string getRandomMessage() {
    if (getFrenchEnabled()) {
        auto msgs = getFrenchMessages();
        return msgs[rand() % msgs.size()];
    }

    auto msgs = getEnglishMessages();
    return msgs[rand() % msgs.size()];
}

static void playReminderSound() {
    auto soundPath = CCFileUtils::sharedFileUtils()->fullPathForFilename("reminder.mp3", false);
    if (!soundPath.empty()) {
        FMODAudioEngine::sharedEngine()->playEffect(soundPath.c_str());
    }
    else {
        FMODAudioEngine::sharedEngine()->playEffect("achievement_01.ogg");
    }
}

static void showReminderNow() {
    auto scene = CCDirector::sharedDirector()->getRunningScene();
    if (!scene) return;

    auto existing = scene->getChildByTag(987654);
    if (existing) {
        existing->removeFromParentAndCleanup(true);
    }

    auto notif = TopReminderNotification::create(getRandomMessage());
    if (notif) {
        notif->setTag(987654);
        scene->addChild(notif, 999999);
    }

    playReminderSound();
}

class $modify(GoOutsideBroScheduler, CCScheduler) {
    void update(float dt) {
        CCScheduler::update(dt);

        auto mod = Mod::get();

        double totalTracked = mod->getSavedValue<double>("tracked-total-seconds", 0.0);
        totalTracked += dt;
        mod->setSavedValue("tracked-total-seconds", totalTracked);

        if (getTestMode()) {
            g_testTimer += dt;
            if (g_testTimer >= 5.f) {
                g_testTimer = 0.f;
                showReminderNow();
            }
            return;
        }

        if (!getNotificationsEnabled()) {
            return;
        }

        g_reminderTimer += dt;

        float needed = getReminderSeconds();
        if (needed <= 0.f) return;

        if (g_reminderTimer >= needed) {
            g_reminderTimer = 0.f;
            showReminderNow();
        }
    }
};

class $modify(GoOutsideBroMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto timeSprite = ButtonSprite::create("Time");
        timeSprite->setScale(0.8f);

        auto timeButton = CCMenuItemSpriteExtra::create(
            timeSprite,
            this,
            menu_selector(GoOutsideBroMenuLayer::onOpenPlaytime)
        );

        auto menu = CCMenu::create();
        menu->setPosition(CCPointZero);
        timeButton->setPosition(ccp(winSize.width - 50.f, winSize.height / 2 - 35.f));

        menu->addChild(timeButton);
        this->addChild(menu, 100);

        return true;
    }

    void onOpenPlaytime(CCObject*) {
        double totalTracked = Mod::get()->getSavedValue<double>("tracked-total-seconds", 0.0);

        std::string text =
            "Tracked playtime by this mod:\n" +
            formatTime(totalTracked) +
            "\n\nThis is the time tracked since the mod was installed,\nnot the full account lifetime playtime.";

        FLAlertLayer::create(
            "Playtime Tracker",
            text,
            "OK"
        )->show();
    }
};

$on_mod(Loaded) {
}
