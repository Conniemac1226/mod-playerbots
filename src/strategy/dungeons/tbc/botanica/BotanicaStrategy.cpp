#include "BotanicaStrategy.h"
#include "BotanicaMultipliers.h"

void BotanicaStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Commander Sarannis
    triggers.push_back(new TriggerNode("sarannis resonance",
        NextAction::array(0, new NextAction("sarannis resonance dispel", ACTION_DISPEL + 2), nullptr)));
    
    triggers.push_back(new TriggerNode("sarannis reinforcements",
        NextAction::array(0, new NextAction("sarannis reinforcements", ACTION_RAID + 2), nullptr)));
    
    // High Botanist Freywinn
    triggers.push_back(new TriggerNode("freywinn frayers",
        NextAction::array(0, new NextAction("freywinn frayer priority", ACTION_RAID + 3), nullptr)));
    
    triggers.push_back(new TriggerNode("freywinn tranquility",
        NextAction::array(0, new NextAction("freywinn tranquility", ACTION_RAID + 1), nullptr)));
    
    // Laj
    triggers.push_back(new TriggerNode("laj allergic reaction",
        NextAction::array(0, new NextAction("laj allergic reaction", ACTION_MOVE + 4), nullptr)));
    
    triggers.push_back(new TriggerNode("laj teleport",
        NextAction::array(0, new NextAction("laj teleport position", ACTION_MOVE + 3), nullptr)));
    
    // Thorngrin the Tender
    triggers.push_back(new TriggerNode("thorngrin sacrifice",
        NextAction::array(0, new NextAction("thorngrin sacrifice", ACTION_DISPEL + 1), nullptr)));
    
    triggers.push_back(new TriggerNode("thorngrin hellfire",
        NextAction::array(0, new NextAction("thorngrin hellfire", ACTION_MOVE + 5), nullptr)));
    
    triggers.push_back(new TriggerNode("thorngrin enrage",
        NextAction::array(0, new NextAction("thorngrin enrage", ACTION_DISPEL), nullptr)));
    
    // Warp Splinter
    triggers.push_back(new TriggerNode("warp splinter war stomp",
        NextAction::array(0, new NextAction("warp splinter war stomp", ACTION_MOVE + 5), nullptr)));
    
    triggers.push_back(new TriggerNode("warp splinter arcane volley",
        NextAction::array(0, new NextAction("warp splinter arcane volley", ACTION_INTERRUPT), nullptr)));
}

void BotanicaStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new SarannisMultiplier(botAI));
    multipliers.push_back(new FreywinnMultiplier(botAI));
    multipliers.push_back(new LajMultiplier(botAI));
    multipliers.push_back(new ThorngrinMultiplier(botAI));
    multipliers.push_back(new WarpSplinterMultiplier(botAI));
}