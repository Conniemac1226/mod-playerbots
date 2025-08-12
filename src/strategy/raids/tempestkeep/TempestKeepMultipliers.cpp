#include "TempestKeepMultipliers.h"
#include "TempestKeepActions.h"
#include "PlayerbotAI.h"
#include "Player.h"
#include "Unit.h"

float AlarFlameQuillsMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return 1.0f;

    // During Flame Quills, prioritize movement actions
    if (boss->FindCurrentSpellBySpellId(SPELL_FLAME_QUILLS))
    {
        if (dynamic_cast<AlarFlameQuillsAction*>(action))
            return 1.0f;
            
        // Reduce priority of other actions during quills
        return 0.5f;
    }

    return 1.0f;
}

float AlarDiveBombMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return 1.0f;

    // During dive bomb, prioritize escape
    if (boss->HasAura(SPELL_DIVE_BOMB_VISUAL) || !boss->IsVisible())
    {
        if (dynamic_cast<AlarDiveBombAction*>(action))
            return 1.0f;
            
        // Reduce other actions during dive
        return 0.3f;
    }

    return 1.0f;
}

float VoidReaverPoundingMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return 1.0f;

    // During Pounding, prioritize range
    if (boss->FindCurrentSpellBySpellId(SPELL_POUNDING))
    {
        if (dynamic_cast<VoidReaverPoundingAction*>(action))
            return 1.0f;
            
        // Reduce melee actions
        return bot->IsWithinMeleeRange(boss) ? 0.2f : 0.8f;
    }

    return 1.0f;
}

float VoidReaverPositionMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return 1.0f;

    // Prioritize positioning actions
    if (dynamic_cast<VoidReaverPositionAction*>(action))
    {
        // Tank positioning is critical after knock away
        if (botAI->IsMainTank(bot))
            return 1.0f;
            
        // Ranged positioning for arcane orb
        return 0.9f;
    }

    return 1.0f;
}

float SolarianWrathMultiplier::GetValue(Action* action)
{
    // If we have Wrath debuff, prioritize spreading
    if (bot->HasAura(SPELL_WRATH_OF_THE_ASTROMANCER))
    {
        if (dynamic_cast<SolarianWrathAction*>(action))
            return 1.0f;
            
        // Reduce stacking actions
        return 0.5f;
    }

    return 1.0f;
}

float SolarianBlindingLightMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return 1.0f;

    // During Blinding Light cast, prioritize facing away
    if (boss->FindCurrentSpellBySpellId(SPELL_BLINDING_LIGHT))
    {
        if (dynamic_cast<SolarianBlindingLightAction*>(action))
            return 1.0f;
            
        // Reduce other actions to focus on positioning
        return 0.7f;
    }

    return 1.0f;
}

float KaelthasGravityLapseMultiplier::GetValue(Action* action)
{
    // During Gravity Lapse, movement is different
    if (bot->HasAura(SPELL_TK_GRAVITY_LAPSE))
    {
        if (dynamic_cast<KaelthasGravityLapseAction*>(action))
            return 1.0f;
            
        // Reduce normal movement actions
        return 0.3f;
    }

    return 1.0f;
}