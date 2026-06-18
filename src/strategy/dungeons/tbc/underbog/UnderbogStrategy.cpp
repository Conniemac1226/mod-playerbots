#include "UnderbogStrategy.h"
#include "Ai/Dungeon/DungeonAutoPull.h"
#include "UnderbogTriggers.h"
#include "UnderbogMultipliers.h"

void UnderbogStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    DungeonAutoPull::AddDefaultPullTrigger(triggers);

    // Hungarfen
    triggers.push_back(new TriggerNode(
        "mushroom nearby",
        NextAction::array(0, new NextAction("avoid mushroom explosion", ACTION_EMERGENCY), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "foul spores active",
        NextAction::array(0, new NextAction("avoid foul spores", ACTION_MOVE + 3), nullptr)));

    // Ghazan
    triggers.push_back(new TriggerNode(
        "acid breath casting",
        NextAction::array(0, new NextAction("avoid acid breath", ACTION_MOVE + 2), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "tail sweep danger",
        NextAction::array(0, new NextAction("avoid tail sweep", ACTION_MOVE + 1), nullptr)));

    // Swamplord Musel'ek
    triggers.push_back(new TriggerNode(
        "windcaller claw active",
        NextAction::array(0, new NextAction("attack windcaller claw", ACTION_RAID + 4), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "freezing trap incoming",
        NextAction::array(0, new NextAction("avoid freezing trap", ACTION_MOVE + 3), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "hunters mark active",
        NextAction::array(0, new NextAction("dispel hunters mark", ACTION_DISPEL + 1), nullptr)));

    // The Black Stalker
    triggers.push_back(new TriggerNode(
        "levitate active",
        NextAction::array(0, new NextAction("position for levitate", ACTION_MOVE + 2), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "spore strider active",
        NextAction::array(0, new NextAction("attack spore strider", ACTION_RAID + 5), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "chain lightning casting",
        NextAction::array(0, new NextAction("interrupt chain lightning", ACTION_INTERRUPT + 1), 
                           new NextAction("spread for chain lightning", ACTION_MOVE + 1), nullptr)));

    triggers.push_back(new TriggerNode(
        "static charge active",
        NextAction::array(0, new NextAction("avoid static charge", ACTION_MOVE + 4), nullptr)));
}

void UnderbogStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // CRITICAL: Block DpsAssist when adds present - prevents boss/add oscillation
    // RESEARCHED: Following HallsOfLightningStrategy.cpp:37-40 pattern
    multipliers.push_back(new BlackStalkerAddMultiplier(botAI));
}
