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

    // WotLK pattern - check for any totem type present
    uint32 totemIds[] = { NPC_NOVA_TOTEM, NPC_HEALING_WARD, NPC_EARTHGRAB_TOTEM, NPC_STONESKIN_TOTEM };
    
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat())
        {
            for (uint32 totemId : totemIds)
            {
                if (unit->GetEntry() == totemId)
                {
                    return 0.0f; // Block DpsAssist when any totem present
                }
            }
        }
    }
    return 1.0f;
}