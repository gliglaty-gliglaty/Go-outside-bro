#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

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
protected:
    CCSprite* m_bg = nullptr;
    CCLabelBMFont* m_label = nullptr;

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

        m_bg = CCSprite::create("square02b_001.png");
        if (m_bg) {
            m_bg->setScaleX(3.8f);
            m_bg->setScaleY(0.9f);
            m_bg->setOpacity(140);
            m_bg->setColor(ccc3(0, 0, 0));
            this->addChild(m_bg);
        }

        m_label = CCLabelBMFont::create(
            text.c_str(),
            "chatFont.fnt",
            300.f,
            kCCTextAlignmentCenter
        );

        if (m_label) {
            m_label->setScale(0.75f);
            m_label->setPosition(CCPointZero);
            this->addChild(m_label);
        }

        this->setPosition(ccp(winSize.width / 2, winSize.height + 30.f));

        auto moveIn = CCMoveTo::create(0.25f, ccp(winSize.width / 2, winSize.height - 25.f));
        auto delay = CCDelayTime::create(4.5f);
        auto moveOut = CCMoveTo::create(0.25f, ccp(winSize.width / 2, winSize.height + 30.f));
        auto cleanup = CCCallFunc::create(this, callfunc_selector(TopReminderNotification::removeSelf));

        this->runAction(CCSequence::create(moveIn, delay, moveOut, cleanup, nullptr));
    }

    void removeSelf() {
        this->removeFromParentAndCleanup(true);
    }
};

class ReminderNode : public CCNode {
protected:
    float m_timer = 0.f;
    float m_testTimer = 0.f;

    bool m_lastNotificationsEnabled = true;
    bool m_lastTestMode = false;
    bool m_lastFrench = false;
    bool m_lastUseHours = false;
    int m_lastAmount = 30;

    std::vector<std::string> getFrenchMessages() {
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
            "Petit rappel : le soleil existe encore.",
            "Va boire de l'eau, champion.",
            "Une pause ne va pas detruire ton skill.",
            "Le dehors n'est pas si effrayant, promis.",
            "Fais une pause, ton dos te remercie.",
            "L'herbe te manque probablement.",
            "Va respirer un peu hors de ta chambre.",
            "Ton ecran ne va pas s'enfuir.",
            "Tu peux revenir apres une petite pause.",
            "Il est peut-etre temps de cligner des yeux.",
            "Va marcher un peu, juste un peu.",
            "Ton corps demande une pause.",
            "Le monde exterieur pense a toi.",
            "Va voir la lumiere du jour.",
            "Pause rapide obligatoire, bro.",
            "Prends soin de toi au lieu de grind non-stop.",
            "Tes jambes existent aussi, utilise-les.",
            "Va t'etirer un peu.",
            "Le jeu sera encore la dans cinq minutes.",
            "Tu peux souffler un instant.",
            "C'est l'heure d'une mini pause.",
            "Le salon, la cuisine, le balcon... choisis une destination.",
            "Essaie de ne pas fusionner avec ta chaise.",
            "Ton corps n'est pas un setup RGB.",
            "Va faire un tour avant de devenir un PNJ.",
            "Le dehors t'appelle doucement.",
            "Bro, prends une pause avant de devenir legendaire dans le mauvais sens.",
            "Tu peux poser le jeu une minute, promis.",
            "Une petite pause = meilleur grind ensuite.",
            "Va detendre tes epaules.",
            "Tu as pense a boire quelque chose ?",
            "Va respirer de l'air frais.",
            "Tu farmes les niveaux, mais pense a te reposer.",
            "Le grind c'est bien, la survie aussi.",
            "Petit check-up : tu es encore vivant ?",
            "Va te lever de cette chaise, heros.",
            "Le mode statue est active depuis trop longtemps.",
            "Une pause maintenant, c'est une victoire plus tard.",
            "Va faire quelques pas.",
            "Laisse ton cerveau charger un peu.",
            "Une pause courte peut sauver ton humeur."
        };
    }

    std::vector<std::string> getEnglishMessages() {
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
            "Reminder: the sun still exists.",
            "Go drink some water, champion.",
            "A short break won't ruin your skill.",
            "The outside world is not that scary, promise.",
            "Take a break, your back will thank you.",
            "The grass probably misses you.",
            "Go breathe outside your room for a bit.",
            "Your screen is not going anywhere.",
            "You can come back after a short break.",
            "It may be time to blink again.",
            "Go walk around a little.",
            "Your body is asking for a break.",
            "The outside world is thinking about you.",
            "Go look at daylight for a moment.",
            "Quick break required, bro.",
            "Take care of yourself instead of grinding non-stop.",
            "Your legs still exist, use them.",
            "Go stretch a little.",
            "The game will still be here in five minutes.",
            "You can breathe for a second.",
            "It's time for a mini break.",
            "The living room, kitchen, balcony... choose a destination.",
            "Try not to merge with your chair.",
            "Your body is not an RGB setup.",
            "Go outside before you become an NPC.",
            "The outside is calling you softly.",
            "Bro, take a break before becoming legendary in the wrong way.",
            "You can put the game down for a minute, trust me.",
            "A short break means better grinding later.",
            "Go relax your shoulders.",
            "Have you thought about drinking something?",
            "Go get some fresh air.",
            "You're farming levels, but remember to rest too.",
            "Grinding is good, surviving is also good.",
            "Quick check: are you still alive?",
            "Stand up from that chair, hero.",
            "Statue mode has been active for too long.",
            "A break now is a win later.",
            "Go take a few steps.",
            "Let your brain load for a bit.",
            "A short break can save your mood."
        };
    }

    bool getNotificationsEnabled() {
        auto mod = Mod::get();
        if (mod->hasSetting("enable-notifications")) {
            return mod->getSettingValue<bool>("enable-notifications");
        }
        return true;
    }

    bool getTestMode() {
        auto mod = Mod::get();
        if (mod->hasSetting("test-mode")) {
            return mod->getSettingValue<bool>("test-mode");
        }
        return false;
    }

    bool getFrenchEnabled() {
        auto mod = Mod::get();
        if (mod->hasSetting("language-french")) {
            return mod->getSettingValue<bool>("language-french");
        }
        return false;
    }

    bool getUseHours() {
        auto mod = Mod::get();
        if (mod->hasSetting("reminder-hours")) {
            return mod->getSettingValue<bool>("reminder-hours");
        }
        return false;
    }

    int getReminderAmount() {
        auto mod = Mod::get();
        if (mod->hasSetting("reminder-amount")) {
            int amount = static_cast<int>(mod->getSettingValue<int64_t>("reminder-amount"));
            if (amount < 1) amount = 1;
            return amount;
        }
        return 30;
    }

    float getReminderSeconds() {
        int amount = getReminderAmount();
        bool useHours = getUseHours();

        if (useHours) {
            return static_cast<float>(amount * 3600);
        }

        return static_cast<float>(amount * 60);
    }

    std::string getRandomMessage() {
        if (getFrenchEnabled()) {
            auto msgs = getFrenchMessages();
            return msgs[rand() % msgs.size()];
        }

        auto msgs = getEnglishMessages();
        return msgs[rand() % msgs.size()];
    }

    void playReminderSound() {
        auto soundPath = (CCFileUtils::sharedFileUtils()->fullPathForFilename("reminder.mp3"));
        FMODAudioEngine::sharedEngine()->playEffect(soundPath.c_str());
    }

    void showReminderNow() {
        auto message = getRandomMessage();

        auto scene = CCDirector::sharedDirector()->getRunningScene();
        if (scene) {
            auto notif = TopReminderNotification::create(message);
            if (notif) {
                scene->addChild(notif, 9999);
            }
        }

        playReminderSound();
    }

    void refreshSettingsState() {
        bool notificationsEnabled = getNotificationsEnabled();
        bool testMode = getTestMode();
        bool french = getFrenchEnabled();
        bool useHours = getUseHours();
        int amount = getReminderAmount();

        if (
            notificationsEnabled != m_lastNotificationsEnabled ||
            testMode != m_lastTestMode ||
            french != m_lastFrench ||
            useHours != m_lastUseHours ||
            amount != m_lastAmount
        ) {
            if (
                testMode != m_lastTestMode ||
                useHours != m_lastUseHours ||
                amount != m_lastAmount
            ) {
                m_timer = 0.f;
                m_testTimer = 0.f;
            }

            m_lastNotificationsEnabled = notificationsEnabled;
            m_lastTestMode = testMode;
            m_lastFrench = french;
            m_lastUseHours = useHours;
            m_lastAmount = amount;

            log::info("Settings updated live");
        }
    }

public:
    static ReminderNode* create() {
        auto ret = new ReminderNode();
        if (ret && ret->init()) {
            ret->autorelease();
            ret->schedule(schedule_selector(ReminderNode::tick), 1.0f);

            ret->m_lastNotificationsEnabled = ret->getNotificationsEnabled();
            ret->m_lastTestMode = ret->getTestMode();
            ret->m_lastFrench = ret->getFrenchEnabled();
            ret->m_lastUseHours = ret->getUseHours();
            ret->m_lastAmount = ret->getReminderAmount();

            return ret;
        }

        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    static double getTrackedTotalTime() {
        return Mod::get()->getSavedValue<double>("tracked-total-seconds", 0.0);
    }

    void tick(float dt) {
        refreshSettingsState();

        m_timer += dt;

        auto mod = Mod::get();

        double totalTracked = mod->getSavedValue<double>("tracked-total-seconds", 0.0);
        totalTracked += dt;
        mod->setSavedValue("tracked-total-seconds", totalTracked);

        if (getTestMode()) {
            m_testTimer += dt;
            if (m_testTimer >= 5.f) {
                m_testTimer = 0.f;
                showReminderNow();
            }
            return;
        }

        if (!getNotificationsEnabled()) {
            return;
        }

        float needed = getReminderSeconds();
        if (needed <= 0.f) return;

        if (m_timer >= needed) {
            m_timer = 0.f;
            showReminderNow();
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
        double totalTracked = ReminderNode::getTrackedTotalTime();

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
    log::info("Go outside, bro! loaded");
}
