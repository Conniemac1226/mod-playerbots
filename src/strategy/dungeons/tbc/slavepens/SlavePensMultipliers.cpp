#include "SlavePensMultipliers.h"
#include "SlavePensActions.h"

float SlavePensMultiplier::GetValue(Action* action)
{
    // Following proven pattern of minimal multipliers to avoid priority violations
    // All priorities are set directly in strategy triggers
    return 1.0f;
}

float MennuTotemMultiplier::GetValue(Action* action)
{
    if (!action || action->getName() != "dps assist")
        return 1.0f;

    Player* bot = botAI->GetBot();
    if (!bot)
        return 1.0f;

    Unit* boss = bot->FindNearestCreature(NPC_MENNU_THE_BETRAYER, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return 1.0f;

    // WotLK-style spawned add check: only block when killable totems are present.
    GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (ObjectGuid const& target : npcs)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsAlive() && IsMennuAttackableTotemEntry(unit->GetEntry()))
        {
            return 0.0f; // Block DpsAssist when killable totems are present
        }
    }

    return 1.0f;
}
