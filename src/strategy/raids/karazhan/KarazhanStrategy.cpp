#include "KarazhanStrategy.h"
#include "KarazhanMultipliers.h"

void KarazhanStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Attumen the Huntsman
    triggers.push_back(new TriggerNode("attumen engaged",
        NextAction::array(0, new NextAction("attumen position", ACTION_MOVE + 2), nullptr)));
    triggers.push_back(new TriggerNode("attumen mounted",
        NextAction::array(0, new NextAction("attumen avoid charge", ACTION_MOVE + 3), nullptr)));
    triggers.push_back(new TriggerNode("attumen charge danger",
        NextAction::array(0, new NextAction("attumen avoid charge", ACTION_EMERGENCY), nullptr)));
    triggers.push_back(new TriggerNode("attumen shadowcleave",
        NextAction::array(0, new NextAction("attumen position", ACTION_MOVE + 1), nullptr)));
    
    // Moroes
    triggers.push_back(new TriggerNode("moroes engaged",
        NextAction::array(0, new NextAction("moroes position", ACTION_HIGH + 2), nullptr)));
    triggers.push_back(new TriggerNode("moroes adds",
        NextAction::array(0, new NextAction("moroes focus adds", ACTION_EMERGENCY - 1), nullptr)));
    triggers.push_back(new TriggerNode("moroes garrote",
        NextAction::array(0, new NextAction("moroes crowd control", ACTION_HIGH + 5), nullptr)));
    
    // Maiden of Virtue
    triggers.push_back(new TriggerNode("maiden engaged",
        NextAction::array(0, new NextAction("maiden holy ground", ACTION_MOVE + 1), nullptr)));
    triggers.push_back(new TriggerNode("maiden repentance",
        NextAction::array(0, new NextAction("maiden repentance", ACTION_DISPEL + 1), nullptr)));
    triggers.push_back(new TriggerNode("maiden holy ground",
        NextAction::array(0, new NextAction("maiden holy ground", ACTION_MOVE + 3), nullptr)));
    
    // Opera Event
    triggers.push_back(new TriggerNode("opera engaged",
        NextAction::array(0, new NextAction("opera position", ACTION_HIGH + 2), nullptr)));
    triggers.push_back(new TriggerNode("opera oz engaged",
        NextAction::array(0, new NextAction("opera focus target", ACTION_HIGH + 3), nullptr)));
    triggers.push_back(new TriggerNode("opera romulo julianne",
        NextAction::array(0, new NextAction("opera focus target", ACTION_HIGH + 3), nullptr)));
    triggers.push_back(new TriggerNode("opera wolf",
        NextAction::array(0, new NextAction("opera position", ACTION_HIGH + 2), nullptr)));
    
    // Curator
    triggers.push_back(new TriggerNode("curator engaged",
        NextAction::array(0, new NextAction("curator evocation", ACTION_MOVE + 1), nullptr)));
    triggers.push_back(new TriggerNode("curator flare",
        NextAction::array(0, new NextAction("curator flare", ACTION_EMERGENCY - 1), nullptr)));
    triggers.push_back(new TriggerNode("curator evocation",
        NextAction::array(0, new NextAction("curator evocation", ACTION_HIGH + 5), nullptr)));
    
    // Shade of Aran
    triggers.push_back(new TriggerNode("aran engaged",
        NextAction::array(0, new NextAction("aran dragons breath", ACTION_MOVE + 1), nullptr)));
    triggers.push_back(new TriggerNode("aran flame wreath",
        NextAction::array(0, new NextAction("aran flame wreath", ACTION_EMERGENCY + 1), nullptr)));
    triggers.push_back(new TriggerNode("aran blizzard",
        NextAction::array(0, new NextAction("aran blizzard", ACTION_MOVE + 5), nullptr)));
    triggers.push_back(new TriggerNode("aran dragons breath",
        NextAction::array(0, new NextAction("aran dragons breath", ACTION_MOVE + 4), nullptr)));
    
    // Terestian Illhoof
    triggers.push_back(new TriggerNode("illhoof engaged",
        NextAction::array(0, new NextAction("illhoof imps", ACTION_HIGH + 2), nullptr)));
    triggers.push_back(new TriggerNode("illhoof demon chains",
        NextAction::array(0, new NextAction("illhoof demon chains", ACTION_EMERGENCY - 1), nullptr)));
    triggers.push_back(new TriggerNode("illhoof imps",
        NextAction::array(0, new NextAction("illhoof imps", ACTION_HIGH + 4), nullptr)));
    
    // Netherspite
    triggers.push_back(new TriggerNode("netherspite engaged",
        NextAction::array(0, new NextAction("netherspite beam", ACTION_MOVE + 1), nullptr)));
    triggers.push_back(new TriggerNode("netherspite beams",
        NextAction::array(0, new NextAction("netherspite beam", ACTION_HIGH + 3), nullptr)));
    triggers.push_back(new TriggerNode("netherspite void zone",
        NextAction::array(0, new NextAction("netherspite void zone", ACTION_MOVE + 5), nullptr)));
    
    // Prince Malchezaar
    triggers.push_back(new TriggerNode("malchezaar engaged",
        NextAction::array(0, new NextAction("malchezaar infernal", ACTION_MOVE + 1), nullptr)));
    triggers.push_back(new TriggerNode("malchezaar infernal",
        NextAction::array(0, new NextAction("malchezaar infernal", ACTION_MOVE + 5), nullptr)));
    triggers.push_back(new TriggerNode("malchezaar enfeeble",
        NextAction::array(0, new NextAction("malchezaar enfeeble", ACTION_EMERGENCY - 1), nullptr)));
    
    // Nightbane
    triggers.push_back(new TriggerNode("nightbane engaged",
        NextAction::array(0, new NextAction("nightbane position", ACTION_MOVE + 2), nullptr)));
    triggers.push_back(new TriggerNode("nightbane air phase",
        NextAction::array(0, new NextAction("nightbane air phase", ACTION_HIGH + 3), nullptr)));
    triggers.push_back(new TriggerNode("nightbane charred earth",
        NextAction::array(0, new NextAction("nightbane charred earth", ACTION_MOVE + 5), nullptr)));
        
    // Utility triggers
    triggers.push_back(new TriggerNode("moroes gouge",
        NextAction::array(0, new NextAction("moroes tank swap", ACTION_EMERGENCY), nullptr)));
    triggers.push_back(new TriggerNode("karazhan interrupt needed",
        NextAction::array(0, new NextAction("karazhan interrupt rotation", ACTION_HIGH + 6), nullptr)));
    triggers.push_back(new TriggerNode("karazhan dispel needed",
        NextAction::array(0, new NextAction("karazhan dispel", ACTION_DISPEL + 2), nullptr)));
    triggers.push_back(new TriggerNode("chess event active",
        NextAction::array(0, new NextAction("chess event move", ACTION_MOVE + 1), 
                            new NextAction("chess event ability", ACTION_HIGH + 2), nullptr)));
}

void KarazhanStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    // multipliers.push_back(new AttumenMultiplier(botAI));
}