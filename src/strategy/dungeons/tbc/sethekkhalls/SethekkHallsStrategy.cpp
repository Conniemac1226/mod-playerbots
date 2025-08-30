#include "SethekkHallsStrategy.h"
#include "SethekkHallsMultipliers.h"

void TbcDungeonSHStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Trash: Sethekk Spirit - Flee from ghost spawned when Sethekk Prophets die
    // High priority to ensure bots immediately flee from these dangerous ghosts
    triggers.push_back(new TriggerNode("sethekk spirit nearby",
             NextAction::array(0, new NextAction("flee sethekk spirit", ACTION_EMERGENCY + 3), nullptr)));
    
    // Trash: Time-Lost Controller - Charming Totem Priority
    // Using high base priority instead of multipliers to match WotLK patterns
    triggers.push_back(new TriggerNode("charming totem spawned",
             NextAction::array(0, 
                new NextAction("attack charming totem", ACTION_EMERGENCY + 2),
                new NextAction("continue fight with charmed ally", ACTION_MOVE + 1),
                nullptr)));
    
    triggers.push_back(new TriggerNode("time lost controller casting totem",
             NextAction::array(0, new NextAction("interrupt controller", ACTION_INTERRUPT + 1), nullptr)));
    
    // Boss: Talon King Ikiss - Simple distance-based AoE avoidance (researched from boss script)
    // When boss has Arcane Bubble (9438), move away from boss location
    triggers.push_back(new TriggerNode("ikiss arcane explosion cast",
             NextAction::array(0, new NextAction("ikiss move away", ACTION_MOVE + 5), nullptr)));
    
    // When boss loses Arcane Bubble, return to combat
    triggers.push_back(new TriggerNode("ikiss arcane explosion ended",
             NextAction::array(0, new NextAction("ikiss return position", ACTION_MOVE + 3), nullptr)));

    // Boss: Anzu - Priority add targeting (banish phases at 66% and 33%)
    // Moderate priority to kill broods without blocking healing/combat
    triggers.push_back(new TriggerNode("brood of anzu nearby",
             NextAction::array(0, new NextAction("attack brood of anzu", ACTION_NORMAL + 5), nullptr)));
    
    // NOTE: Anzu spell bomb and cyclone mechanics cannot be implemented without guessing
    // No proven patterns exist for detecting spell bomb targets or cyclone range mechanics
    // Only implementing what can be proven: Brood of Anzu targeting (proven from totem pattern)
}

void TbcDungeonSHStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    // Removed multipliers to prevent priority stacking issues
    // Following WotLK dungeon patterns that don't use multipliers for target switching
}