#include "SethekkHallsStrategy.h"
#include "SethekkHallsMultipliers.h"

void TbcDungeonSHStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Trash: Sethekk Spirit - Flee from ghost spawned when Sethekk Prophets die
    triggers.push_back(new TriggerNode("sethekk spirit nearby",
             NextAction::array(0, new NextAction("flee sethekk spirit", ACTION_EMERGENCY + 3), nullptr)));
    
    // Trash: Time-Lost Controller - Charming Totem Priority
    triggers.push_back(new TriggerNode("charming totem spawned",
             NextAction::array(0, 
                new NextAction("attack charming totem", ACTION_EMERGENCY + 2),
                new NextAction("continue fight with charmed ally", ACTION_MOVE + 1),
                nullptr)));
    
    triggers.push_back(new TriggerNode("time lost controller casting totem",
             NextAction::array(0, new NextAction("interrupt controller", ACTION_INTERRUPT + 1), nullptr)));
    
    triggers.push_back(new TriggerNode("ikiss arcane explosion cast",
             NextAction::array(0, new NextAction("ikiss move away", ACTION_MOVE + 5), nullptr)));
    
    triggers.push_back(new TriggerNode("ikiss arcane explosion ended",
             NextAction::array(0, new NextAction("ikiss return position", ACTION_MOVE + 3), nullptr)));

    // Boss: Anzu - Priority add targeting (banish phases at 66% and 33%)
    triggers.push_back(new TriggerNode("brood of anzu nearby",
             NextAction::array(0, new NextAction("attack brood of anzu", ACTION_NORMAL + 5), nullptr)));
    
}

void TbcDungeonSHStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    // CRITICAL: Block DpsAssist when adds present - prevents boss/add oscillation
    // RESEARCHED: Following HallsOfLightningStrategy.cpp:37-40 pattern
    multipliers.push_back(new CharmingTotemMultiplier(botAI));
    multipliers.push_back(new BroodOfAnzuMultiplier(botAI));
}