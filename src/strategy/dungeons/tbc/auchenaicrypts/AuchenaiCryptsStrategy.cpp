#include "AuchenaiCryptsStrategy.h"
#include "AuchenaiCryptsMultipliers.h"

void TbcDungeonACStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    
    // Boss: Shirrak the Dead Watcher - Focus Fire avoidance (critical mechanic)
    // RESEARCHED: From boss_shirrak_the_dead_watcher.cpp lines 120-149
    // Boss summons Focus Fire creature, then casts Fiery Blast 3 times over 3-4 seconds
    triggers.push_back(new TriggerNode("shirrak focus fire spawned",
             NextAction::array(0, new NextAction("shirrak focus fire avoid", ACTION_MOVE + 5), nullptr)));
    
    // When Focus Fire phase ends, return to optimal combat positioning
    triggers.push_back(new TriggerNode("shirrak focus fire ended",
             NextAction::array(0, new NextAction("shirrak return position", ACTION_MOVE + 5), nullptr)));
}

void TbcDungeonACStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new ShirrakFocusFireMultiplier(botAI));
}