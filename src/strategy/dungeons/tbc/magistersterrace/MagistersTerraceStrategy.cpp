#include "MagistersTerraceStrategy.h"
#include "MagistersTerraceMultipliers.h"

void MagistersTerraceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Selin Fireheart
    triggers.push_back(new TriggerNode(
        "selin fireheart fel explosion",
        NextAction::array(0, new NextAction("avoid fel explosion", ACTION_MOVE + 3), NULL)
    ));

    triggers.push_back(new TriggerNode(
        "fel crystal nearby",
        NextAction::array(0, new NextAction("attack fel crystal", ACTION_EMERGENCY + 2), NULL)
    ));

    // Vexallus
    triggers.push_back(new TriggerNode(
        "vexallus pure energy spawned",
        NextAction::array(0, new NextAction("attack pure energy", ACTION_EMERGENCY + 1), NULL)
    ));

    // Priestess Delrissa
    triggers.push_back(new TriggerNode(
        "delrissa add active",
        NextAction::array(0, new NextAction("attack delrissa add", ACTION_EMERGENCY), NULL)
    ));

    triggers.push_back(new TriggerNode(
        "delrissa add active",
        NextAction::array(0, new NextAction("interrupt delrissa helper", ACTION_INTERRUPT + 2), NULL)
    ));

    triggers.push_back(new TriggerNode(
        "delrissa add active", 
        NextAction::array(0, new NextAction("delrissa dispel handling", ACTION_NORMAL + 5), NULL)
    ));

    // Kael'thas
    triggers.push_back(new TriggerNode(
        "kaelthas casting pyroblast",
        NextAction::array(0, new NextAction("interrupt kaelthas pyroblast", ACTION_EMERGENCY + 3), NULL)
    ));

    triggers.push_back(new TriggerNode(
        "kaelthas casting gravity lapse",
        NextAction::array(0, new NextAction("avoid gravity lapse", ACTION_MOVE + 1), NULL)
    ));

    triggers.push_back(new TriggerNode(
        "kaelthas arcane sphere nearby",
        NextAction::array(0, new NextAction("flee arcane sphere", ACTION_MOVE + 2), NULL)
    ));

    triggers.push_back(new TriggerNode(
        "kaelthas flamestrike",
        NextAction::array(0, new NextAction("avoid flamestrike", ACTION_MOVE + 4), NULL)
    ));
}

void MagistersTerraceStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new MagistersTerraceMultiplier(botAI));
}