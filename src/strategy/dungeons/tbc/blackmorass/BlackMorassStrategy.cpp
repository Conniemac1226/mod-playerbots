#include "BlackMorassStrategy.h"
#include "BlackMorassActions.h"
#include "BlackMorassTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

BlackMorassStrategy::BlackMorassStrategy(PlayerbotAI* botAI) : Strategy(botAI)
{
}

void BlackMorassStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Aeonus cleave avoidance (high priority)
    triggers.push_back(new TriggerNode(
        "aeonus cleave nearby",
        NextAction::array(0, new NextAction("aeonus avoid cleave", ACTION_MOVE + 2), nullptr)));

    // Aeonus positioning for tanks
    triggers.push_back(new TriggerNode(
        "aeonus engaged",
        NextAction::array(0, new NextAction("aeonus position", ACTION_MOVE + 1), nullptr)));

    // Chrono Lord Deja AoE avoidance
    triggers.push_back(new TriggerNode(
        "time lapse nearby", 
        NextAction::array(0, new NextAction("avoid time lapse", ACTION_MOVE + 1), nullptr)));
        
    triggers.push_back(new TriggerNode(
        "arcane discharge nearby",
        NextAction::array(0, new NextAction("avoid arcane discharge", ACTION_MOVE + 1), nullptr)));

    // Temporus positioning
    triggers.push_back(new TriggerNode(
        "wing buffet nearby",
        NextAction::array(0, new NextAction("avoid wing buffet", ACTION_MOVE + 1), nullptr)));
}