#include "SteamvaultStrategy.h"
#include "Ai/Dungeon/DungeonAutoPull.h"
#include "SteamvaultMultipliers.h"

void SteamvaultStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    DungeonAutoPull::AddDefaultPullTrigger(triggers);

    // Hydromancer Thespia
    triggers.push_back(new TriggerNode(
        "thespia lightning cloud",
        NextAction::array(0, new NextAction("avoid lightning cloud", ACTION_MOVE + 3), NULL)
    ));

    triggers.push_back(new TriggerNode(
        "thespia lung burst",
        NextAction::array(0, new NextAction("dispel lung burst", ACTION_MOVE + 2), NULL)
    ));

    triggers.push_back(new TriggerNode(
        "thespia water elemental active",
        NextAction::array(0, new NextAction("attack water elemental", ACTION_MOVE + 4), NULL)
    ));

    // Mekgineer Steamrigger
    triggers.push_back(new TriggerNode(
        "steamrigger shrink ray",
        NextAction::array(0, new NextAction("dispel shrink ray", ACTION_MOVE + 1), NULL)
    ));

    triggers.push_back(new TriggerNode(
        "steamrigger saw blade",
        NextAction::array(0, new NextAction("avoid saw blade", ACTION_EMERGENCY), NULL)
    ));

    triggers.push_back(new TriggerNode(
        "steamrigger electrified net",
        NextAction::array(0, new NextAction("remove electrified net", ACTION_MOVE + 2), NULL)
    ));

    triggers.push_back(new TriggerNode(
        "steamrigger mechanic active",
        NextAction::array(0, new NextAction("attack steamrigger mechanic", ACTION_EMERGENCY + 2), NULL)
    ));

    // Warlord Kalithresh
    triggers.push_back(new TriggerNode(
        "kalithresh spell reflection",
        NextAction::array(0, new NextAction("stop casting spell reflection", ACTION_EMERGENCY + 3), NULL)
    ));

    triggers.push_back(new TriggerNode(
        "kalithresh spell reflection ended",
        NextAction::array(0, new NextAction("resume attack after spell reflection", ACTION_NORMAL + 1), NULL)
    ));

    triggers.push_back(new TriggerNode(
        "kalithresh impale",
        NextAction::array(0, new NextAction("heal impale target", ACTION_EMERGENCY + 1), NULL)
    ));

    triggers.push_back(new TriggerNode(
        "kalithresh naga distiller active",
        NextAction::array(0, new NextAction("attack naga distiller", ACTION_EMERGENCY + 2), NULL)
    ));

    triggers.push_back(new TriggerNode(
        "kalithresh channeling rage",
        NextAction::array(0, new NextAction("interrupt distiller channel", ACTION_EMERGENCY + 3), NULL)
    ));
}

void SteamvaultStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new SteamvaultMultiplier(botAI));
    // WotLK-style targeted suppression: only block generic DPS logic while
    // the relevant add or reflect mechanic is actually active.
    multipliers.push_back(new ThespiaWaterElementalMultiplier(botAI));
    multipliers.push_back(new SteamriggerMechanicMultiplier(botAI));
    multipliers.push_back(new KalithreshSpellReflectionMultiplier(botAI));
    multipliers.push_back(new KalithreshDistillerMultiplier(botAI));
}
