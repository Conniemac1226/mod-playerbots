#include "EscapeFromDurnholdeStrategy.h"
#include "EscapeFromDurnholdeTriggers.h"
#include "EscapeFromDurnholdeActions.h"
#include "SharedDefines.h"
#include "Strategy.h"

void EscapeFromDurnholdeStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Thrall NPC healing triggers (highest priority)
    triggers.push_back(new TriggerNode(
        "thrall critical health",
        NextAction::array(0, new NextAction("thrall emergency heal", ACTION_EMERGENCY + 5), NULL)));
    
    triggers.push_back(new TriggerNode(
        "thrall low health", 
        NextAction::array(0, new NextAction("heal thrall", ACTION_CRITICAL_HEAL + 2), NULL)));
    
    // Lieutenant Drake whirlwind avoidance
    triggers.push_back(new TriggerNode(
        "lieutenant drake whirlwind",
        NextAction::array(0, new NextAction("efd avoid whirlwind", ACTION_MOVE + 3), NULL)));
    
    // Return to position after whirlwind ends
    triggers.push_back(new TriggerNode(
        "efd return position",
        NextAction::array(0, new NextAction("efd return position", ACTION_MOVE + 2), NULL)));
    
    // Captain Skarloc hammer of justice dispel
    triggers.push_back(new TriggerNode(
        "captain skarloc hammer of justice",
        NextAction::array(0, new NextAction("dispel hammer of justice", ACTION_DISPEL + 1), NULL)));
    
    // Epoch Hunter positioning and magic disruption
    triggers.push_back(new TriggerNode(
        "epoch hunter sand breath",
        NextAction::array(0, new NextAction("epoch hunter position", ACTION_MOVE + 2), NULL)));
    
    triggers.push_back(new TriggerNode(
        "epoch hunter magic disruption aura",
        NextAction::array(0, new NextAction("cancel magic disruption", ACTION_INTERRUPT + 1), NULL)));
}

void EscapeFromDurnholdeStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    // Basic strategy without custom multipliers
}