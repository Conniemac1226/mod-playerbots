#include "KarazhanStrategy.h"
#include "KarazhanMultipliers.h"
#include "Log.h"

void KarazhanStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Attumen the Huntsman - removed problematic engaged trigger
    triggers.push_back(new TriggerNode("attumen mounted",
        NextAction::array(0, new NextAction("attumen avoid charge", ACTION_MOVE + 3), nullptr)));
    triggers.push_back(new TriggerNode("attumen charge danger",
        NextAction::array(0, new NextAction("attumen avoid charge", ACTION_EMERGENCY), nullptr)));
    triggers.push_back(new TriggerNode("attumen shadowcleave",
        NextAction::array(0, new NextAction("attumen position", ACTION_MOVE + 1), nullptr)));
    
    // Moroes - MULTI-NPC encounter - fixed with WotLK AttackAction pattern
    triggers.push_back(new TriggerNode("moroes adds",
        NextAction::array(0, new NextAction("moroes focus adds", ACTION_HIGH + 2), nullptr)));
    
    // Maiden of Virtue - SINGLE BOSS - keeping triggers
    triggers.push_back(new TriggerNode("maiden repentance",
        NextAction::array(0, new NextAction("maiden repentance", ACTION_DISPEL + 1), nullptr)));
    triggers.push_back(new TriggerNode("maiden holy ground",
        NextAction::array(0, new NextAction("maiden holy ground", ACTION_MOVE + 3), nullptr)));
    
    // Opera Event - MULTI-NPC encounter - fixed with WotLK AttackAction pattern
    triggers.push_back(new TriggerNode("opera oz engaged",
        NextAction::array(0, new NextAction("opera focus target", ACTION_HIGH + 2), nullptr)));
    triggers.push_back(new TriggerNode("opera romulo julianne",
        NextAction::array(0, new NextAction("opera focus target", ACTION_HIGH + 2), nullptr)));
    triggers.push_back(new TriggerNode("opera wolf",
        NextAction::array(0, new NextAction("opera focus target", ACTION_HIGH + 2), nullptr)));
    
    // Curator - SINGLE BOSS - keeping triggers
    triggers.push_back(new TriggerNode("curator flare",
        NextAction::array(0, new NextAction("curator flare", ACTION_EMERGENCY - 1), nullptr)));
    triggers.push_back(new TriggerNode("curator evocation",
        NextAction::array(0, new NextAction("curator evocation", ACTION_HIGH + 5), nullptr)));
    
    // Shade of Aran - SINGLE BOSS - keeping triggers
    triggers.push_back(new TriggerNode("aran flame wreath",
        NextAction::array(0, new NextAction("aran flame wreath", ACTION_EMERGENCY + 1), nullptr)));
    triggers.push_back(new TriggerNode("aran blizzard",
        NextAction::array(0, new NextAction("aran blizzard", ACTION_MOVE + 5), nullptr)));
    triggers.push_back(new TriggerNode("aran dragons breath",
        NextAction::array(0, new NextAction("aran dragons breath", ACTION_MOVE + 4), nullptr)));
    
    // Terestian Illhoof - MULTI-NPC encounter (with adds) - fixed with WotLK AttackAction pattern
    triggers.push_back(new TriggerNode("illhoof demon chains",
        NextAction::array(0, new NextAction("illhoof demon chains", ACTION_EMERGENCY - 1), nullptr)));
    triggers.push_back(new TriggerNode("illhoof imps",
        NextAction::array(0, new NextAction("illhoof imps", ACTION_HIGH + 4), nullptr)));
    
    // Netherspite - MULTI-ROLE beam assignments per original PR strategy
    triggers.push_back(new TriggerNode("karazhan netherspite",
        NextAction::array(0,
            new NextAction("karazhan netherspite block red beam", ACTION_EMERGENCY + 8),
            new NextAction("karazhan netherspite block blue beam", ACTION_EMERGENCY + 8),
            new NextAction("karazhan netherspite block green beam", ACTION_EMERGENCY + 8),
            new NextAction("karazhan netherspite avoid beam and void zone", ACTION_EMERGENCY + 7),
            new NextAction("karazhan netherspite banish phase avoid void zone", ACTION_RAID + 1),
            nullptr)));
    
    // Prince Malchezaar - SPAWNED ADDS ENCOUNTER - per CLAUDE.md:645-658
    triggers.push_back(new TriggerNode("malchezaar infernal",
        NextAction::array(0, new NextAction("malchezaar infernal", ACTION_RAID + 4), nullptr)));
    triggers.push_back(new TriggerNode("malchezaar enfeeble",
        NextAction::array(0, new NextAction("malchezaar enfeeble", ACTION_EMERGENCY - 1), nullptr)));
    
    // Nightbane - SPAWNED ADDS ENCOUNTER - per CLAUDE.md:645-658
    triggers.push_back(new TriggerNode("nightbane skeleton",
        NextAction::array(0, new NextAction("nightbane skeleton", ACTION_RAID + 3), nullptr)));
    triggers.push_back(new TriggerNode("nightbane air phase",
        NextAction::array(0, new NextAction("nightbane air phase", ACTION_HIGH + 3), nullptr)));
    triggers.push_back(new TriggerNode("nightbane charred earth",
        NextAction::array(0, new NextAction("nightbane charred earth", ACTION_MOVE + 5), nullptr)));
        
    // Utility triggers - keeping as they're generic
    triggers.push_back(new TriggerNode("moroes gouge",
        NextAction::array(0, new NextAction("moroes tank swap", ACTION_EMERGENCY), nullptr)));
    triggers.push_back(new TriggerNode("karazhan interrupt needed",
        NextAction::array(0, new NextAction("karazhan interrupt rotation", ACTION_INTERRUPT + 1), nullptr)));
    triggers.push_back(new TriggerNode("karazhan dispel needed",
        NextAction::array(0, new NextAction("karazhan dispel", ACTION_DISPEL + 2), nullptr)));
    triggers.push_back(new TriggerNode("chess event active",
        NextAction::array(0, new NextAction("chess event move", ACTION_MOVE + 1), 
                            new NextAction("chess event ability", ACTION_HIGH + 2), nullptr)));
}

void KarazhanStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    // CRITICAL: Block DpsAssist when adds present - prevents boss/add oscillation  
    // Following HallsOfLightningStrategy.cpp:37-40 pattern per CLAUDE.md:782-786
    multipliers.push_back(new MoroesAddMultiplier(botAI));
    multipliers.push_back(new CuratorAddMultiplier(botAI));
    multipliers.push_back(new IllhoofAddMultiplier(botAI));
    multipliers.push_back(new MalchezaarAddMultiplier(botAI));
    multipliers.push_back(new NightbaneAddMultiplier(botAI));
    multipliers.push_back(new NetherspiteBlueAndGreenBeamMultiplier(botAI));
    multipliers.push_back(new NetherspiteRedBeamMultiplier(botAI));
    
    // multipliers.push_back(new AttumenMultiplier(botAI));
}
