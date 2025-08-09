#include "SethekkHallsStrategy.h"
#include "SethekkHallsMultipliers.h"

void TbcDungeonSHStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Trash: Time-Lost Controller - Charming Totem Priority
    // Using high base priority instead of multipliers to match WotLK patterns
    triggers.push_back(new TriggerNode("charming totem spawned",
             NextAction::array(0, new NextAction("attack charming totem", ACTION_EMERGENCY + 2), nullptr)));
    
    triggers.push_back(new TriggerNode("time lost controller casting totem",
             NextAction::array(0, new NextAction("interrupt controller", ACTION_INTERRUPT + 1), nullptr)));
    
    // Boss: Talon King Ikiss - Simple distance-based AoE avoidance (researched from boss script)
    // When boss has Arcane Bubble (9438), move away from boss location
    triggers.push_back(new TriggerNode("ikiss arcane explosion cast",
             NextAction::array(0, new NextAction("ikiss move away", ACTION_MOVE + 5), nullptr)));
    
    // When boss loses Arcane Bubble, return to combat
    triggers.push_back(new TriggerNode("ikiss arcane explosion ended",
             NextAction::array(0, new NextAction("ikiss return position", ACTION_MOVE + 3), nullptr)));
}

void TbcDungeonSHStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    // Removed multipliers to prevent priority stacking issues
    // Following WotLK dungeon patterns that don't use multipliers for target switching
}