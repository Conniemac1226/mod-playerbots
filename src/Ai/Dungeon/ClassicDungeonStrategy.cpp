#include "ClassicDungeonStrategy.h"
#include "DungeonAutoPull.h"

void ClassicDungeonStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    DungeonAutoPull::AddDefaultPullTrigger(triggers);
}
