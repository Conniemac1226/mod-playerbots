#include "Playerbots.h"
#include "BotanicaMultipliers.h"
#include "BotanicaActions.h"
#include "SharedDefines.h"

float SarannisMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "commander sarannis");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return 1.0f;
    }
    
    return 1.0f;
}

float FreywinnMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "high botanist freywinn");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return 1.0f;
    }
    
    return 1.0f;
}

float LajMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "laj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return 1.0f;
    }
    
    return 1.0f;
}

float ThorngrinMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "thorngrin the tender");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return 1.0f;
    }
    
    return 1.0f;
}

float WarpSplinterMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warp splinter");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return 1.0f;
    }
    
    return 1.0f;
}