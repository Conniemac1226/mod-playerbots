#include "SlavePensStrategy.h"
#include "Ai/Dungeon/DungeonAutoPull.h"
#include "SlavePensTriggers.h"
#include "SlavePensMultipliers.h"

void SlavePensStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    DungeonAutoPull::AddDefaultPullTrigger(triggers);

    // Mennu the Betrayer
    triggers.push_back(new TriggerNode(
        "mennu totem active",
        NextAction::array(0, new NextAction("attack mennu totem", ACTION_HIGH + 5), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "mennu lightning bolt cast",
        NextAction::array(0, new NextAction("interrupt mennu lightning bolt", ACTION_INTERRUPT + 1), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "nova totem nearby",
        NextAction::array(0, new NextAction("avoid nova totem", ACTION_EMERGENCY), nullptr)));

    // Rokmar the Crackler
    triggers.push_back(new TriggerNode(
        "ensnaring moss active",
        NextAction::array(0, new NextAction("dispel ensnaring moss", ACTION_DISPEL + 1), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "grievous wound active",
        NextAction::array(0, new NextAction("heal grievous wound", ACTION_EMERGENCY), nullptr)));

    triggers.push_back(new TriggerNode(
        "rokmar frenzy",
        NextAction::array(0, new NextAction("rokmar frenzy", ACTION_DISPEL + 2), nullptr)));

    triggers.push_back(new TriggerNode(
        "water spit incoming",
        NextAction::array(0, new NextAction("avoid water spit", ACTION_MOVE + 2), nullptr)));

    // Quagmirran
    triggers.push_back(new TriggerNode(
        "acid spray casting",
        NextAction::array(0, new NextAction("avoid acid spray", ACTION_MOVE + 2), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "poison bolt volley casting",
        NextAction::array(0, new NextAction("interrupt poison bolt volley", ACTION_INTERRUPT + 1), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "uppercut positioning",
        NextAction::array(0, new NextAction("tank position uppercut", ACTION_MOVE + 1), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "quagmirran cleave positioning",
        NextAction::array(0, new NextAction("avoid quagmirran cleave", ACTION_MOVE + 1), nullptr)));
}

void SlavePensStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new MennuTotemMultiplier(botAI));
}
