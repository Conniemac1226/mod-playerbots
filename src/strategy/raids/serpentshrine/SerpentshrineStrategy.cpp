#include "SerpentshrineStrategy.h"
#include "SerpentshrineMultipliers.h"
#include "Log.h"

void SerpentshrineStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Strategy is loading correctly
    // Tank transition is highest priority - prevents wipes from too many stacks
    triggers.push_back(new TriggerNode(
        "hydross transition needed",
        NextAction::array(0, new NextAction("hydross transition control", ACTION_EMERGENCY + 4), nullptr)));

    // Water Tomb spread is critical to avoid chain stunning
    triggers.push_back(new TriggerNode(
        "hydross water tomb",
        NextAction::array(0, new NextAction("hydross water tomb spread", ACTION_EMERGENCY + 3), nullptr)));

    // Vile Sludge spread to avoid healing reduction
    triggers.push_back(new TriggerNode(
        "hydross vile sludge",
        NextAction::array(0, new NextAction("hydross vile sludge spread", ACTION_EMERGENCY + 2), nullptr)));

    // Adds need to die quickly but after positioning
    triggers.push_back(new TriggerNode(
        "hydross adds",
        NextAction::array(0, new NextAction("hydross kill adds", ACTION_EMERGENCY + 1), nullptr)));

    // Mark management for non-tanks
    triggers.push_back(new TriggerNode(
        "hydross mark of hydross",
        NextAction::array(0, new NextAction("hydross avoid mark of hydross", ACTION_MOVE + 6), nullptr)));

    triggers.push_back(new TriggerNode(
        "hydross mark of corruption",
        NextAction::array(0, new NextAction("hydross avoid mark of corruption", ACTION_MOVE + 6), nullptr)));

    // Tank positioning
    triggers.push_back(new TriggerNode(
        "hydross tank position",
        NextAction::array(0, new NextAction("hydross position tank", ACTION_MOVE + 5), nullptr)));
    
    // The Lurker Below triggers
    // Spout is highest priority - must jump in water
    triggers.push_back(new TriggerNode(
        "lurker spout",
        NextAction::array(0, new NextAction("lurker spout", ACTION_EMERGENCY + 5), nullptr)));
    
    // Whirl avoidance for melee
    triggers.push_back(new TriggerNode(
        "lurker whirl",
        NextAction::array(0, new NextAction("lurker whirl avoid", ACTION_EMERGENCY + 2), nullptr)));
    
    // Geyser spread
    triggers.push_back(new TriggerNode(
        "lurker geyser",
        NextAction::array(0, new NextAction("lurker geyser spread", ACTION_MOVE + 5), nullptr)));
    
    // Kill adds during submerge phase
    triggers.push_back(new TriggerNode(
        "lurker adds",
        NextAction::array(0, new NextAction("lurker kill adds", ACTION_EMERGENCY + 1), nullptr)));
    
    // Positioning
    triggers.push_back(new TriggerNode(
        "lurker position",
        NextAction::array(0, new NextAction("lurker position", ACTION_MOVE + 3), nullptr)));
    
    // Leotheras the Blind triggers
    // Whirlwind avoidance is critical
    triggers.push_back(new TriggerNode(
        "leotheras whirlwind",
        NextAction::array(0, new NextAction("leotheras whirlwind", ACTION_EMERGENCY + 4), nullptr)));
    
    // Chaos blast spread for demon form
    triggers.push_back(new TriggerNode(
        "leotheras chaos blast",
        NextAction::array(0, new NextAction("leotheras chaos blast", ACTION_EMERGENCY + 2), nullptr)));
    
    // Inner demon must be killed immediately
    triggers.push_back(new TriggerNode(
        "leotheras inner demon",
        NextAction::array(0, new NextAction("leotheras inner demon", ACTION_EMERGENCY + 5), nullptr)));
    
    // Shadow at 15% health
    triggers.push_back(new TriggerNode(
        "leotheras shadow",
        NextAction::array(0, new NextAction("leotheras shadow", ACTION_EMERGENCY + 2), nullptr)));
    
    // Positioning
    triggers.push_back(new TriggerNode(
        "leotheras position",
        NextAction::array(0, new NextAction("leotheras position", ACTION_MOVE + 3), nullptr)));
    
    // Fathom-Lord Karathress triggers
    // Advisors must die in correct order
    triggers.push_back(new TriggerNode(
        "karathress advisors",
        NextAction::array(0, new NextAction("karathress advisors", ACTION_EMERGENCY + 3), nullptr)));

    triggers.push_back(new TriggerNode(
        "karathress cyclone",
        NextAction::array(0, new NextAction("karathress cyclone", ACTION_EMERGENCY + 4), nullptr)));

    // Force natural falling when cyclone ends
    triggers.push_back(new TriggerNode(
        "karathress cyclone ended",
        NextAction::array(0, new NextAction("karathress cyclone fall", ACTION_EMERGENCY + 5), nullptr)));

    // Continuous spread for ranged DPS to prevent cyclone clustering
    triggers.push_back(new TriggerNode(
        "karathress spread",
        NextAction::array(0, new NextAction("karathress spread", ACTION_NORMAL + 5), nullptr)));

    // Totems are high priority
    triggers.push_back(new TriggerNode(
        "karathress totems",
        NextAction::array(0, new NextAction("karathress totems", ACTION_EMERGENCY + 4), nullptr)));
    
    // Cataclysmic bolt avoidance for mana users
    triggers.push_back(new TriggerNode(
        "karathress cataclysmic bolt",
        NextAction::array(0, new NextAction("karathress cataclysmic bolt", ACTION_EMERGENCY + 2), nullptr)));
    
    // Sear nova spread
    triggers.push_back(new TriggerNode(
        "karathress sear nova",
        NextAction::array(0, new NextAction("karathress sear nova", ACTION_EMERGENCY + 2), nullptr)));
    
    // Tidal surge avoidance
    triggers.push_back(new TriggerNode(
        "karathress tidal surge",
        NextAction::array(0, new NextAction("karathress tidal surge", ACTION_EMERGENCY + 3), nullptr)));
    
    // Morogrim Tidewalker triggers
    // Tidal Wave frontal cone avoidance
    triggers.push_back(new TriggerNode(
        "morogrim tidal wave",
        NextAction::array(0, new NextAction("morogrim tidal wave", ACTION_EMERGENCY + 3), nullptr)));
    
    // Watery Grave healing priority
    triggers.push_back(new TriggerNode(
        "morogrim watery grave",
        NextAction::array(0, new NextAction("morogrim watery grave", ACTION_EMERGENCY + 2), nullptr)));
    
    // Murlocs are highest priority - must die fast
    triggers.push_back(new TriggerNode(
        "morogrim murlocs",
        NextAction::array(0, new NextAction("morogrim murlocs", ACTION_EMERGENCY + 5), nullptr)));
    
    // Globules in phase 2
    triggers.push_back(new TriggerNode(
        "morogrim globules",
        NextAction::array(0, new NextAction("morogrim globules", ACTION_EMERGENCY + 4), nullptr)));
    
    // Positioning behind boss
    triggers.push_back(new TriggerNode(
        "morogrim position",
        NextAction::array(0, new NextAction("morogrim position", ACTION_MOVE + 4), nullptr)));
    
    // Lady Vashj triggers
    // Phase 2 adds are highest priority
    triggers.push_back(new TriggerNode(
        "vashj coilfang elite",
        NextAction::array(0, new NextAction("vashj coilfang elite", ACTION_EMERGENCY + 5), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "vashj coilfang strider",
        NextAction::array(0, new NextAction("vashj coilfang strider", ACTION_EMERGENCY + 4), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "vashj enchanted elemental",
        NextAction::array(0, new NextAction("vashj enchanted elemental", ACTION_EMERGENCY + 3), nullptr)));
    
    triggers.push_back(new TriggerNode(
        "vashj tainted elemental",
        NextAction::array(0, new NextAction("vashj tainted elemental", ACTION_EMERGENCY + 3), nullptr)));
    
    // Phase 3 sporebats
    triggers.push_back(new TriggerNode(
        "vashj sporebat",
        NextAction::array(0, new NextAction("vashj sporebat", ACTION_EMERGENCY + 4), nullptr)));
    
    // Shock Blast avoidance
    triggers.push_back(new TriggerNode(
        "vashj shock blast",
        NextAction::array(0, new NextAction("vashj shock blast", ACTION_EMERGENCY + 2), nullptr)));
    
    // Static Charge spread
    triggers.push_back(new TriggerNode(
        "vashj static charge",
        NextAction::array(0, new NextAction("vashj static charge", ACTION_EMERGENCY + 2), nullptr)));
    
    // Entangle removal
    triggers.push_back(new TriggerNode(
        "vashj entangle",
        NextAction::array(0, new NextAction("vashj entangle", ACTION_DISPEL + 3), nullptr)));
    
    // Tainted Core movement (Phase 2)
    triggers.push_back(new TriggerNode(
        "vashj tainted core",
        NextAction::array(0, new NextAction("vashj tainted core", ACTION_EMERGENCY + 1), nullptr)));
    
    // Positioning
    triggers.push_back(new TriggerNode(
        "vashj position",
        NextAction::array(0, new NextAction("vashj position", ACTION_MOVE + 3), nullptr)));
}

void SerpentshrineStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new HydrossResistanceMultiplier(botAI));
    multipliers.push_back(new HydrossAddsMultiplier(botAI));
    multipliers.push_back(new HydrossTankMultiplier(botAI));
    multipliers.push_back(new LeotherasThreatHoldMultiplier(botAI));
    multipliers.push_back(new SerpentshrinePriorityMultiplier(botAI));
}
