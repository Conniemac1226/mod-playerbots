#include "BlackTempleStrategy.h"
#include "BlackTempleMultipliers.h"

void RaidBtStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // General Black Temple triggers will go here for trash/environment
}

void RaidBtStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // General Black Temple multipliers
}

void RaidBtNajentusStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // High priority: Avoid impaled targets
    triggers.push_back(new TriggerNode(
        "najentus impaled nearby",
        NextAction::array(0, new NextAction("najentus avoid impaled", ACTION_EMERGENCY), NULL)));

    // High priority: Throw spine when shield is up
    triggers.push_back(new TriggerNode(
        "najentus tidal shield up",
        NextAction::array(0, new NextAction("najentus throw spine", ACTION_EMERGENCY - 5.0f), NULL)));

    // Medium priority: Pick up spines
    triggers.push_back(new TriggerNode(
        "najentus spine available",
        NextAction::array(0, new NextAction("najentus spine pickup", ACTION_MOVE + 5.0f), NULL)));

    // Medium priority: Spread for Tidal Burst
    triggers.push_back(new TriggerNode(
        "najentus tidal burst soon",
        NextAction::array(0, new NextAction("najentus tidal burst position", ACTION_MOVE + 3.0f), NULL)));
}

void RaidBtNajentusStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new NajentusTargetMultiplier(botAI));
    multipliers.push_back(new NajentusImpaledMultiplier(botAI));
}

void RaidBtSupremusStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Phase tracking
    triggers.push_back(new TriggerNode(
        "supremus engaged",
        NextAction::array(0, new NextAction("supremus phase check", ACTION_HIGH), NULL)));

    // Kite phase - highest priority
    triggers.push_back(new TriggerNode(
        "supremus kite phase",
        NextAction::array(0, new NextAction("supremus kite", ACTION_EMERGENCY + 1.0f), NULL)));

    // Volcano avoidance - emergency priority
    triggers.push_back(new TriggerNode(
        "supremus volcano nearby",
        NextAction::array(0, new NextAction("supremus avoid volcano", ACTION_EMERGENCY), NULL)));

    // Molten flame avoidance
    triggers.push_back(new TriggerNode(
        "supremus flame nearby",
        NextAction::array(0, new NextAction("supremus avoid flame", ACTION_EMERGENCY - 1.0f), NULL)));

    // Tank phase positioning
    triggers.push_back(new TriggerNode(
        "supremus tank phase",
        NextAction::array(0, new NextAction("supremus hateful position", ACTION_MOVE + 2.0f), NULL)));
}

void RaidBtSupremusStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Add Supremus-specific multipliers if needed
}

void RaidBtShadeOfAkamaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Phase 1: Kill channelers first
    triggers.push_back(new TriggerNode(
        "shade channeler active",
        NextAction::array(0, new NextAction("shade channeler target", ACTION_EMERGENCY - 1.0f), NULL)));

    // Add management
    triggers.push_back(new TriggerNode(
        "shade adds active",
        NextAction::array(0, new NextAction("shade adds target", ACTION_HIGH + 5.0f), NULL)));

    // Protect Akama when his health is low
    triggers.push_back(new TriggerNode(
        "shade akama low health",
        NextAction::array(0, new NextAction("shade protect akama", ACTION_EMERGENCY), NULL)));

    // Positioning for phase 1
    triggers.push_back(new TriggerNode(
        "shade phase one",
        NextAction::array(0, new NextAction("shade position", ACTION_MOVE + 2.0f), NULL)));

    // Positioning for phase 2
    triggers.push_back(new TriggerNode(
        "shade phase two",
        NextAction::array(0, new NextAction("shade position", ACTION_MOVE + 1.0f), NULL)));
}

void RaidBtShadeOfAkamaStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Add Shade-specific multipliers if needed
}

void RaidBtTeronGorefiendStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Shadow of Death - highest priority
    triggers.push_back(new TriggerNode(
        "teron shadow of death",
        NextAction::array(0, new NextAction("teron shadow of death", ACTION_EMERGENCY + 2.0f), NULL)));

    // Ghost form - special abilities on constructs
    triggers.push_back(new TriggerNode(
        "teron ghost form",
        NextAction::array(0, new NextAction("teron ghost form", ACTION_EMERGENCY + 1.0f), NULL)));

    // Doom Blossom avoidance - emergency priority
    triggers.push_back(new TriggerNode(
        "teron doom blossom nearby",
        NextAction::array(0, new NextAction("teron avoid doom blossom", ACTION_EMERGENCY), NULL)));

    // Incinerate spread
    triggers.push_back(new TriggerNode(
        "teron incinerate",
        NextAction::array(0, new NextAction("teron incinerate spread", ACTION_MOVE + 5.0f), NULL)));

    // Crushing Shadows spread
    triggers.push_back(new TriggerNode(
        "teron crushing shadows",
        NextAction::array(0, new NextAction("teron crushing shadows spread", ACTION_MOVE + 3.0f), NULL)));
}

void RaidBtTeronGorefiendStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Add Teron-specific multipliers if needed
}

void RaidBtGurtoggBloodboilStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Positioning for Bloodboil mechanic
    triggers.push_back(new TriggerNode(
        "gurtogg bloodboil engaged",
        NextAction::array(0, new NextAction("gurtogg bloodboil position", ACTION_MOVE + 2.0f), NULL)));

    // Fel Rage target - highest priority
    triggers.push_back(new TriggerNode(
        "gurtogg fel rage target",
        NextAction::array(0, new NextAction("gurtogg fel rage target", ACTION_EMERGENCY + 3.0f), NULL)));

    // Fel Geyser avoidance - emergency
    triggers.push_back(new TriggerNode(
        "gurtogg fel geyser nearby",
        NextAction::array(0, new NextAction("gurtogg fel geyser avoid", ACTION_EMERGENCY), NULL)));

    // Arcing Smash avoidance
    triggers.push_back(new TriggerNode(
        "gurtogg arcing smash",
        NextAction::array(0, new NextAction("gurtogg arcing smash avoid", ACTION_EMERGENCY - 1.0f), NULL)));

    // Fel Acid Breath avoidance
    triggers.push_back(new TriggerNode(
        "gurtogg fel acid breath",
        NextAction::array(0, new NextAction("gurtogg fel acid breath avoid", ACTION_EMERGENCY - 1.0f), NULL)));

    // Acidic Wound tank swap
    triggers.push_back(new TriggerNode(
        "gurtogg acidic wound high",
        NextAction::array(0, new NextAction("gurtogg acidic wound swap", ACTION_HIGH + 5.0f), NULL)));
}

void RaidBtGurtoggBloodboilStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Add Gurtogg-specific multipliers if needed
}

// Reliquary of Souls
void RaidBtReliquaryOfSoulsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Phase 1: Essence of Suffering
    triggers.push_back(new TriggerNode(
        "reliquary phase one",
        NextAction::array(0, new NextAction("reliquary suffering position", ACTION_MOVE + 1.0f), NULL)));

    // Phase 2: Essence of Desire
    triggers.push_back(new TriggerNode(
        "reliquary phase two",
        NextAction::array(0, new NextAction("reliquary desire action", ACTION_HIGH + 2.0f), NULL)));

    // Phase 3: Essence of Anger - Spite avoidance
    triggers.push_back(new TriggerNode(
        "reliquary spite",
        NextAction::array(0, new NextAction("reliquary spite avoid", ACTION_EMERGENCY + 2.0f), NULL)));

    // Phase transitions
    triggers.push_back(new TriggerNode(
        "reliquary phase transition",
        NextAction::array(0, new NextAction("reliquary phase transition", ACTION_MOVE), NULL)));

    // Enslaved Soul priority targeting
    triggers.push_back(new TriggerNode(
        "reliquary enslaved soul active",
        NextAction::array(0, new NextAction("reliquary enslaved soul", ACTION_HIGH + 3.0f), NULL)));

    // Soul Scream spread for Phase 3
    triggers.push_back(new TriggerNode(
        "reliquary soul scream",
        NextAction::array(0, new NextAction("reliquary soul scream spread", ACTION_EMERGENCY + 1.0f), NULL)));
}

void RaidBtReliquaryOfSoulsStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Add Reliquary-specific multipliers if needed
}

// Mother Shahraz
void RaidBtMotherShahrazStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Tank positioning for Saber Lash
    triggers.push_back(new TriggerNode(
        "shahraz saber lash tank",
        NextAction::array(0, new NextAction("shahraz saber lash position", ACTION_MOVE + 2.0f), NULL)));

    // Fatal Attraction spreading
    triggers.push_back(new TriggerNode(
        "shahraz fatal attraction",
        NextAction::array(0, new NextAction("shahraz fatal attraction", ACTION_EMERGENCY + 2.0f), NULL)));

    // Beam avoidance (spread from beam targets)
    triggers.push_back(new TriggerNode(
        "shahraz beam target",
        NextAction::array(0, new NextAction("shahraz beam avoid", ACTION_EMERGENCY + 1.0f), NULL)));

    // Prismatic Aura resistance adjustment
    triggers.push_back(new TriggerNode(
        "shahraz prismatic aura",
        NextAction::array(0, new NextAction("shahraz prismatic resistance", ACTION_HIGH), NULL)));
}

void RaidBtMotherShahrazStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Add Mother Shahraz-specific multipliers if needed
}