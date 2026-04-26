#include "ShatteredHallsStrategy.h"
#include "Ai/Dungeon/DungeonAutoPull.h"
#include "ShatteredHallsMultipliers.h"

void ShatteredHallsStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    DungeonAutoPull::AddDefaultPullTrigger(triggers);

    // Gauntlet flame arrow avoidance - High priority
    triggers.push_back(new TriggerNode("flame arrow gauntlet",
        NextAction::array(0, new NextAction("avoid flame arrow fire", ACTION_EMERGENCY - 5), nullptr)));
    
    // Grand Warlock Nethekurse
    triggers.push_back(new TriggerNode("nethekurse shadow fissure",
        NextAction::array(0, new NextAction("avoid shadow fissure", ACTION_MOVE + 5), nullptr)));
    
    triggers.push_back(new TriggerNode("nethekurse dark spin",
        NextAction::array(0, new NextAction("avoid dark spin", ACTION_MOVE + 5), nullptr)));
    
    triggers.push_back(new TriggerNode("nethekurse peons",
        NextAction::array(0, new NextAction("nethekurse peon priority", ACTION_RAID + 2), nullptr)));
    
    // Warbringer O'mrogg
    triggers.push_back(new TriggerNode("omrogg blast wave",
        NextAction::array(0, new NextAction("move from blast wave", ACTION_MOVE + 5), nullptr)));
    
    triggers.push_back(new TriggerNode("omrogg burning maul",
        NextAction::array(0, new NextAction("avoid burning maul", ACTION_MOVE + 4), nullptr)));
    
    // Warchief Kargath Bladefist
    triggers.push_back(new TriggerNode("kargath blade dance",
        NextAction::array(0, new NextAction("avoid blade dance", ACTION_MOVE + 5), nullptr)));
    
    triggers.push_back(new TriggerNode("kargath assassins",
        NextAction::array(0, new NextAction("kill shattered assassins", ACTION_RAID + 2), nullptr)));
}

void ShatteredHallsStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new NethekurseMultiplier(botAI));
    multipliers.push_back(new OmroggMultiplier(botAI));
    multipliers.push_back(new KargathMultiplier(botAI));
}
