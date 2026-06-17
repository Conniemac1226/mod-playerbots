#include "ManaTombsStrategy.h"
#include "Ai/Dungeon/DungeonAutoPull.h"
#include "ManaTombsTriggers.h"
#include "ManaTombsMultipliers.h"

void ManaTombsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    DungeonAutoPull::AddDefaultPullTrigger(triggers);

    // Pandemonius
    triggers.push_back(new TriggerNode(
        "dark shell active",
        NextAction::array(0, new NextAction("avoid dark shell", ACTION_EMERGENCY), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "void blast spread",
        NextAction::array(0, new NextAction("spread void blast", ACTION_MOVE + 2), nullptr)));

    // Tavarok
    triggers.push_back(new TriggerNode(
        "earthquake casting",
        NextAction::array(0, new NextAction("avoid earthquake", ACTION_MOVE + 3), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "crystal prison active",
        NextAction::array(0, new NextAction("break crystal prison", ACTION_HIGH + 5), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "arcing smash danger",
        NextAction::array(0, new NextAction("avoid arcing smash", ACTION_MOVE + 2), nullptr)));

    // Nexus-Prince Shaffar
    triggers.push_back(new TriggerNode(
        "ethereal beacon active",
        NextAction::array(0, new NextAction("attack ethereal beacon", ACTION_HIGH + 6), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "frost nova danger",
        NextAction::array(0, new NextAction("avoid frost nova", ACTION_MOVE + 3), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "shaffar blinked",
        NextAction::array(0, new NextAction("shaffar blink repositioning", ACTION_MOVE + 1), nullptr)));

    // Yor
    triggers.push_back(new TriggerNode(
        "double breath danger",
        NextAction::array(0, new NextAction("avoid double breath", ACTION_MOVE + 2), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "stomp danger",
        NextAction::array(0, new NextAction("avoid stomp", ACTION_MOVE + 3), nullptr)));
}

void ManaTombsStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new PandemoniusDarkShellMultiplier(botAI));
    multipliers.push_back(new EtherealBeaconMultiplier(botAI));
}
