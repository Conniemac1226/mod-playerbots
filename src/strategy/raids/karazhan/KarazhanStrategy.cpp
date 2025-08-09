#include "KarazhanStrategy.h"
#include "KarazhanMultipliers.h"

void KarazhanStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Attumen the Huntsman
    
    // Phase 2: When Attumen spawns at 95% of Midnight's health
    triggers.push_back(new TriggerNode("attumen engaged",
        NextAction::array(0, new NextAction("attumen position", ACTION_MOVE + 2), nullptr)));
    
    // Phase 3: When they mount up (either at 25% health)
    triggers.push_back(new TriggerNode("attumen mounted",
        NextAction::array(0, new NextAction("attumen avoid charge", ACTION_MOVE + 3), nullptr)));
    
    // Avoid being in charge range (8-25 yards) during mounted phase
    triggers.push_back(new TriggerNode("attumen charge danger",
        NextAction::array(0, new NextAction("attumen avoid charge", ACTION_EMERGENCY), nullptr)));
    
    // Avoid frontal cleave from Attumen
    triggers.push_back(new TriggerNode("attumen shadowcleave",
        NextAction::array(0, new NextAction("attumen position", ACTION_MOVE + 1), nullptr)));
}

void KarazhanStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    // multipliers.push_back(new AttumenMultiplier(botAI));
}