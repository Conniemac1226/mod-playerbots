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

float SarannisAddMultiplier::GetValue(Action* action)
{
    if (!action || action->getName() != "dps assist")
        return 1.0f;

    Player* bot = botAI->GetBot();
    if (!bot)
        return 1.0f;

    Unit* boss = AI_VALUE2(Unit*, "find target", "commander sarannis");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return 1.0f;


    // WotLK pattern - check for any Sarannis add present
    const uint32 sarannisAdds[] = {
        NPC_BLOODWARDER_MENDER,     // Priority: Kill healer first
        NPC_BLOODWARDER_RESERVIST   // Priority: Kill DPS second
    };
    
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat())
        {
            for (uint32 addId : sarannisAdds)
            {
                if (unit->GetEntry() == addId)
                {
                    return 0.0f; // Block DpsAssist only while Sarannis encounter adds active
                }
            }
        }
    }
    return 1.0f;
}

float FreywinnAddMultiplier::GetValue(Action* action)
{
    if (!action || action->getName() != "dps assist")
        return 1.0f;

    Player* bot = botAI->GetBot();
    if (!bot)
        return 1.0f;

    // WotLK pattern - check for Frayer add present (only when boss in tree form)
    Unit* boss = AI_VALUE2(Unit*, "find target", "high botanist freywinn");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat() || !boss->HasAura(SPELL_TREE_FORM))
        return 1.0f;
    
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() && unit->GetEntry() == NPC_FRAYER)
        {
            return 0.0f; // Block DpsAssist when Frayer add present during tree form
        }
    }
    return 1.0f;
}
