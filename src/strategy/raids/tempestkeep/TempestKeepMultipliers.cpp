#include "TempestKeepMultipliers.h"
#include "TempestKeepActions.h"
#include "PlayerbotAI.h"
#include "Player.h"
#include "Unit.h"
#include "ChooseTargetActions.h"

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

    // Prioritize ranged positioning actions (tanks handled by normal combat routine)
    if (dynamic_cast<VoidReaverPositionAction*>(action))
        return 1.0f;

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

float AlarFlameBuffetMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return 1.0f;

    // Phase 1 only (boss on platforms)
    if (boss->GetPositionZ() > 15.0f && bot->HasAura(SPELL_FLAME_BUFFET))
    {
        if (dynamic_cast<AlarFlameBuffetAction*>(action))
            return 1.0f;

        // For tanks with Flame Buffet stacks, reduce tank swap blocking actions
        if (botAI->IsTank(bot))
            return 0.6f;
    }

    return 1.0f;
}

float AlarEmberBlastMultiplier::GetValue(Action* action)
{
    // Check for nearby Ember of Al'ar adds that might explode
    if (Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs"))
    {
        GuidVector const npcs = npcsValue->Get();
        for (ObjectGuid const& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && unit->GetEntry() == NPC_EMBER_OF_ALAR)
            {
                if (unit->FindCurrentSpellBySpellId(SPELL_EMBER_BLAST) || bot->GetDistance(unit) < 10.0f)
                {
                    if (dynamic_cast<AlarEmberBlastAction*>(action))
                        return 1.0f;

                    // Reduce other actions when ember blast is imminent
                    return 0.4f;
                }
            }
        }
    }

    return 1.0f;
}

float AlarMeltArmorMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return 1.0f;

    // Phase 2 only (boss on ground) and tank has Melt Armor
    if (boss->GetPositionZ() < 10.0f && bot->HasAura(SPELL_MELT_ARMOR))
    {
        if (dynamic_cast<AlarMeltArmorAction*>(action))
            return 1.0f;

        // For tanks with Melt Armor, prioritize defensive actions
        if (botAI->IsTank(bot))
            return 0.7f;
    }

    return 1.0f;
}

float AlarChargeMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return 1.0f;

    // Phase 2 only (boss on ground)
    if (boss->GetPositionZ() < 10.0f)
    {
        // If boss is casting charge or facing us
        if (boss->FindCurrentSpellBySpellId(SPELL_ALAR_CHARGE) ||
            boss->HasInArc(M_PI_4, bot))
        {
            if (dynamic_cast<AlarChargeAction*>(action))
                return 1.0f;

            // Reduce actions that keep us in charge path
            return 0.5f;
        }
    }

    return 1.0f;
}

// CLAUDE.md: BOSS/ADD OSCILLATION FIX - Block DpsAssist when adds present
float AlarAddMultiplier::GetValue(Action* action)
{
    if (botAI->IsHeal(bot))
        return 1.0f;

    // Check for Ember of Al'ar adds
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    bool addPresent = false;

    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() && unit->GetEntry() == NPC_EMBER_OF_ALAR)
        {
            addPresent = true;
            break;
        }
    }

    // CRITICAL: Block DpsAssist when adds present - prevents boss/add oscillation
    if (addPresent && dynamic_cast<DpsAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

float SolarianAddMultiplier::GetValue(Action* action)
{
    if (botAI->IsHeal(bot))
        return 1.0f;

    // Check for Solarian adds
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    bool addPresent = false;

    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat())
        {
            uint32 entry = unit->GetEntry();
            if (entry == NPC_SOLARIUM_AGENT || entry == NPC_SOLARIUM_PRIEST)
            {
                addPresent = true;
                break;
            }
        }
    }

    // CRITICAL: Block DpsAssist when adds present - prevents boss/add oscillation
    if (addPresent && dynamic_cast<DpsAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

float KaelthasAdvisorMultiplier::GetValue(Action* action)
{
    if (botAI->IsHeal(bot))
        return 1.0f;

    // Check for Kael'thas advisors (pre-existing adds)
    bool advisorPresent = false;

    if (AI_VALUE2(Unit*, "find target", "thaladred the darkener"))
        advisorPresent = true;
    else if (AI_VALUE2(Unit*, "find target", "lord sanguinar"))
        advisorPresent = true;
    else if (AI_VALUE2(Unit*, "find target", "grand astromancer capernian"))
        advisorPresent = true;
    else if (AI_VALUE2(Unit*, "find target", "master engineer telonicus"))
        advisorPresent = true;

    // CRITICAL: Block DpsAssist when advisors present - prevents boss/add oscillation
    if (advisorPresent && dynamic_cast<DpsAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}