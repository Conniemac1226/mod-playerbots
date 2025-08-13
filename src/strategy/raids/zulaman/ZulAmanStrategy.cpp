#include "ZulAmanStrategy.h"
#include "ZulAmanMultipliers.h"

void RaidZaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // General Zul'Aman triggers can go here
}

void RaidZaStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // General Zul'Aman multipliers
}

// --- NALORAKK (Bear) ---
void RaidZaNalorakkStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Brutal Swipe avoidance
    triggers.push_back(new TriggerNode(
        "nalorakk brutal swipe",
        NextAction::array(0, new NextAction("nalorakk brutal swipe avoid", ACTION_EMERGENCY), NULL)));

    // Surge positioning
    triggers.push_back(new TriggerNode(
        "nalorakk surge",
        NextAction::array(0, new NextAction("nalorakk surge position", ACTION_MOVE + 5.0f), NULL)));

    // Bear form positioning
    triggers.push_back(new TriggerNode(
        "nalorakk bear form",
        NextAction::array(0, new NextAction("nalorakk bear form position", ACTION_MOVE + 3.0f), NULL)));
}

void RaidZaNalorakkStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Add Nalorakk-specific multipliers if needed
}

// --- AKIL'ZON (Eagle) ---
void RaidZaAkilzonStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Electrical Storm - highest priority
    triggers.push_back(new TriggerNode(
        "akilzon electrical storm",
        NextAction::array(0, new NextAction("akilzon electrical storm", ACTION_EMERGENCY + 2.0f), NULL)));

    // Static Disruption spread
    triggers.push_back(new TriggerNode(
        "akilzon static disruption",
        NextAction::array(0, new NextAction("akilzon static disruption avoid", ACTION_MOVE + 5.0f), NULL)));

    // Soaring Eagle priority
    triggers.push_back(new TriggerNode(
        "akilzon soaring eagle",
        NextAction::array(0, new NextAction("akilzon eagle target", ACTION_HIGH + 3.0f), NULL)));
}

void RaidZaAkilzonStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Add Akil'zon-specific multipliers if needed
}

// --- JAN'ALAI (Dragonhawk) ---
void RaidZaJanalaiStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Fire Bomb avoidance - emergency
    triggers.push_back(new TriggerNode(
        "janalai fire bomb",
        NextAction::array(0, new NextAction("janalai fire bomb avoid", ACTION_EMERGENCY), NULL)));

    // Hatcher priority - very high
    triggers.push_back(new TriggerNode(
        "janalai hatcher",
        NextAction::array(0, new NextAction("janalai hatcher target", ACTION_HIGH + 5.0f), NULL)));

    // Hatchling management
    triggers.push_back(new TriggerNode(
        "janalai hatchling",
        NextAction::array(0, new NextAction("janalai hatchling target", ACTION_HIGH + 2.0f), NULL)));

    // Fire Wall positioning
    triggers.push_back(new TriggerNode(
        "janalai fire wall",
        NextAction::array(0, new NextAction("janalai fire wall avoid", ACTION_MOVE + 4.0f), NULL)));
}

void RaidZaJanalaiStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Add Jan'alai-specific multipliers if needed
}

// --- HALAZZI (Lynx) ---
void RaidZaHalazziStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Totem priority - highest
    triggers.push_back(new TriggerNode(
        "halazzi totem",
        NextAction::array(0, new NextAction("halazzi totem target", ACTION_EMERGENCY + 1.0f), NULL)));

    // Spirit Lynx priority during split
    triggers.push_back(new TriggerNode(
        "halazzi lynx",
        NextAction::array(0, new NextAction("halazzi lynx target", ACTION_HIGH + 4.0f), NULL)));

    // Saber Lash positioning for tanks
    triggers.push_back(new TriggerNode(
        "halazzi saber lash",
        NextAction::array(0, new NextAction("halazzi saber lash position", ACTION_MOVE + 3.0f), NULL)));
}

void RaidZaHalazziStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Add Halazzi-specific multipliers if needed
}

// --- HEX LORD MALACRASS ---
void RaidZaHexLordStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Drain Power interrupt - critical
    triggers.push_back(new TriggerNode(
        "hex lord drain power",
        NextAction::array(0, new NextAction("hex lord drain power interrupt", ACTION_INTERRUPT), NULL)));

    // Add priority
    triggers.push_back(new TriggerNode(
        "hex lord add",
        NextAction::array(0, new NextAction("hex lord add target", ACTION_HIGH + 4.0f), NULL)));

    // Spirit Bolts spread
    triggers.push_back(new TriggerNode(
        "hex lord spirit bolley",
        NextAction::array(0, new NextAction("hex lord spirit bolley avoid", ACTION_MOVE + 2.0f), NULL)));
}

void RaidZaHexLordStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Add Hex Lord-specific multipliers if needed
}

// --- ZUL'JIN ---
void RaidZaZuljinStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Grievous Throw healing priority
    triggers.push_back(new TriggerNode(
        "zuljin grievous throw",
        NextAction::array(0, new NextAction("zuljin grievous throw heal", ACTION_CRITICAL_HEAL + 5.0f), NULL)));

    // Creeping Paralysis movement
    triggers.push_back(new TriggerNode(
        "zuljin creeping paralysis",
        NextAction::array(0, new NextAction("zuljin creeping paralysis avoid", ACTION_MOVE + 4.0f), NULL)));

    // Feather Vortex targeting
    triggers.push_back(new TriggerNode(
        "zuljin feather vortex",
        NextAction::array(0, new NextAction("zuljin feather vortex target", ACTION_HIGH + 3.0f), NULL)));

    // Cyclone avoidance
    triggers.push_back(new TriggerNode(
        "zuljin cyclone",
        NextAction::array(0, new NextAction("zuljin cyclone avoid", ACTION_EMERGENCY), NULL)));

    // Flame Column avoidance
    triggers.push_back(new TriggerNode(
        "zuljin flame column",
        NextAction::array(0, new NextAction("zuljin flame column avoid", ACTION_MOVE + 5.0f), NULL)));
}

void RaidZaZuljinStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Add Zul'jin-specific multipliers if needed
}