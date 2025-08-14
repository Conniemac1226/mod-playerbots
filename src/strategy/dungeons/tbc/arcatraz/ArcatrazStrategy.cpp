#include "ArcatrazStrategy.h"
#include "ArcatrazMultipliers.h"

void ArcatrazStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Zereketh the Unbound
    triggers.push_back(new TriggerNode("zereketh void zone",
        NextAction::array(0, new NextAction("avoid void zone", ACTION_MOVE + 5), nullptr)));
    
    triggers.push_back(new TriggerNode("zereketh shadow nova",
        NextAction::array(0, new NextAction("avoid shadow nova", ACTION_MOVE + 6), nullptr)));
    
    triggers.push_back(new TriggerNode("zereketh seed of corruption",
        NextAction::array(0, new NextAction("seed of corruption dispel", ACTION_MOVE + 4), nullptr)));
    
    // Dalliah the Doomsayer
    triggers.push_back(new TriggerNode("dalliah whirlwind",
        NextAction::array(0, new NextAction("dalliah whirlwind", ACTION_EMERGENCY + 2), nullptr)));
    
    triggers.push_back(new TriggerNode("dalliah heal",
        NextAction::array(0, new NextAction("dalliah heal interrupt", ACTION_INTERRUPT), nullptr)));
    
    // Wrath-Scryer Soccothrates
    triggers.push_back(new TriggerNode("soccothrates knock away",
        NextAction::array(0, new NextAction("soccothrates knock away", ACTION_MOVE + 5), nullptr)));
    
    triggers.push_back(new TriggerNode("soccothrates charge",
        NextAction::array(0, new NextAction("soccothrates charge", ACTION_MOVE + 4), nullptr)));
    
    triggers.push_back(new TriggerNode("felfire ground",
        NextAction::array(0, new NextAction("avoid felfire ground", ACTION_EMERGENCY + 1), nullptr)));
    
    // Warden Mellichar
    triggers.push_back(new TriggerNode("mellichar adds active",
        NextAction::array(0, new NextAction("attack mellichar adds", ACTION_RAID + 3), nullptr)));
    
    // Harbinger Skyriss
    triggers.push_back(new TriggerNode("skyriss illusion",
        NextAction::array(0, new NextAction("skyriss illusion", ACTION_RAID + 2), nullptr)));
    
    triggers.push_back(new TriggerNode("skyriss fear",
        NextAction::array(0, new NextAction("skyriss fear", ACTION_DISPEL + 2), nullptr)));
    
    triggers.push_back(new TriggerNode("skyriss domination",
        NextAction::array(0, new NextAction("skyriss domination", ACTION_DISPEL + 3), nullptr)));
}

void ArcatrazStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new ZerekethMultiplier(botAI));
    multipliers.push_back(new DalliahMultiplier(botAI));
    multipliers.push_back(new SoccothratesMultiplier(botAI));
    multipliers.push_back(new SkyrissMultiplier(botAI));
}