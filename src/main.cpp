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

    float getReminderSeconds() {
        auto mod = Mod::get();

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

    float getNotificationTime() {
        return 4.5f;
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

            auto notif = Notification::create(
                message,
                NotificationIcon::Info,
                getNotificationTime()
            );

            if (notif) {
                notif->show();
            }

            auto soundPath = (Mod::get()->getConfigDir() / "reminder.mp3").string();
            FMODAudioEngine::sharedEngine()->playEffect(soundPath.c_str());
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
