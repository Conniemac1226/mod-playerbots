#include "SteamvaultMultipliers.h"
#include "SteamvaultTriggers.h"
#include "Unit.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

float SteamvaultMultiplier::GetValue(Action* action)
{
    // No multipliers needed - using proper priority values in strategy
    // Following the pattern from fixed Sethekk Halls, Auchenai Crypts, and Magisters' Terrace
    return 1.0f;
}

float ThespiaWaterElementalMultiplier::GetValue(Action* action)
{
    if (!action || action->getName() != "dps assist")
        return 1.0f;

    Player* bot = botAI->GetBot();
    if (!bot)
        return 1.0f;

    // WotLK pattern - check for Water Elemental add present
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() && unit->GetEntry() == NPC_THESPIA_WATER_ELEMENTAL)
        {
            return 0.0f; // Block DpsAssist when Water Elemental present
        }
    }
    return 1.0f;
}

float SteamriggerMechanicMultiplier::GetValue(Action* action)
{
    if (!action || action->getName() != "dps assist")
        return 1.0f;

    Player* bot = botAI->GetBot();
    if (!bot)
        return 1.0f;

    // WotLK pattern - check for Steamrigger Mechanic add present
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() && unit->GetEntry() == NPC_STEAMRIGGER_MECHANIC)
        {
            return 0.0f; // Block DpsAssist when Steamrigger Mechanic present
        }
    }
    return 1.0f;
}

float KalithreshDistillerMultiplier::GetValue(Action* action)
{
    if (!action || action->getName() != "dps assist")
        return 1.0f;

    Player* bot = botAI->GetBot();
    if (!bot)
        return 1.0f;

    // WotLK pattern - check for Naga Distiller add present
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() && unit->GetEntry() == NPC_NAGA_DISTILLER)
        {
            return 0.0f; // Block DpsAssist when Naga Distiller present
        }
    }
    return 1.0f;
}