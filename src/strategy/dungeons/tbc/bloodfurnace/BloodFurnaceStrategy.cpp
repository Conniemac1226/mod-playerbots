#include "BloodFurnaceStrategy.h"
#include "Ai/Dungeon/DungeonAutoPull.h"
#include "BloodFurnaceMultipliers.h"
#include "BloodFurnaceTriggers.h"
#include "BloodFurnaceActions.h"

void BloodFurnaceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    DungeonAutoPull::AddDefaultPullTrigger(triggers);

    // The Maker triggers - First boss mechanics
    triggers.push_back(new TriggerNode(
        "the maker exploding beaker",
        NextAction::array(0, new NextAction("avoid exploding beaker", ACTION_MOVE + 2), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "the maker domination",
        NextAction::array(0, new NextAction("break domination", ACTION_DISPEL + 1), nullptr)));

    // Broggok triggers - Second boss mechanics
    triggers.push_back(new TriggerNode(
        "broggok poison cloud",
        NextAction::array(0, new NextAction("avoid poison cloud", ACTION_MOVE + 3), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "broggok interrupt poison bolt",
        NextAction::array(0, new NextAction("interrupt poison bolt", ACTION_INTERRUPT + 1), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "broggok slime spray",
        NextAction::array(0, new NextAction("avoid slime spray", ACTION_MOVE + 2), nullptr)));

    // Kelidan triggers - Final boss mechanics
    triggers.push_back(new TriggerNode(
        "kelidan shadowmoon channeler",
        NextAction::array(0, new NextAction("attack shadowmoon channeler", ACTION_HIGH + 5), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "kelidan burning nova",
        NextAction::array(0, new NextAction("avoid burning nova", ACTION_EMERGENCY + 1), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "kelidan interrupt shadow bolt volley",
        NextAction::array(0, new NextAction("interrupt shadow bolt volley", ACTION_INTERRUPT + 1), nullptr)));
    
    // Heroic only trigger
    triggers.push_back(new TriggerNode(
        "kelidan vortex",
        NextAction::array(0, new NextAction("avoid vortex", ACTION_MOVE + 3), nullptr)));
}

void BloodFurnaceStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // CRITICAL: Block DpsAssist when channelers present - prevents boss/add oscillation
    // RESEARCHED: Following HallsOfLightningStrategy.cpp:37-40 pattern
    multipliers.push_back(new KelidanChannelerMultiplier(botAI));
}
