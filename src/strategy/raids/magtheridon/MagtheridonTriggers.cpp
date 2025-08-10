#include "MagtheridonTriggers.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

bool HellfireChannelerNearTrigger::IsActive()
{
    // Only during initial phase
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (magtheridon && !magtheridon->HasAura(SPELL_SHADOW_CAGE))
        return false;
    
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
            
        if (unit->GetEntry() == NPC_HELLFIRE_CHANNELER)
        {
            if (unit->IsInCombat())
                return true;
        }
    }
    
    return false;
}

bool DarkMendingCastTrigger::IsActive()
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
            
        if (unit->GetEntry() == NPC_HELLFIRE_CHANNELER)
        {
            if (unit->FindCurrentSpellBySpellId(SPELL_DARK_MENDING))
            {
                // Check if in interrupt range
                if (bot->GetDistance(unit) <= 30.0f)
                    return true;
            }
        }
    }
    
    return false;
}

bool BlastNovaCastTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;
        
    // Check if Blast Nova is being cast
    if (magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA))
    {
        // Check if we can help with cubes
        if (!bot->HasAura(SPELL_MIND_EXHAUSTION) && !bot->HasAura(SPELL_SHADOW_GRASP))
        {
            uint32 subGroup = bot->GetSubGroup();
            if (subGroup < 5) // First 5 groups handle cubes
                return true;
        }
    }
    
    return false;
}

bool StopCubeChannelTrigger::IsActive()
{
    if (!bot->HasAura(SPELL_SHADOW_GRASP))
        return false;
        
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return true; // Stop if no boss
        
    // Stop if boss is not casting blast nova
    if (!magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA))
        return true;
        
    // Stop if boss is banished (5 cubes active)
    if (magtheridon->HasAura(SPELL_SHADOW_CAGE_STUN))
        return true;
        
    return false;
}

bool QuakeCastTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;
        
    // Detect Quake cast
    if (magtheridon->FindCurrentSpellBySpellId(SPELL_QUAKE))
    {
        // Only trigger if within danger range
        if (bot->GetDistance(magtheridon) <= 30.0f)
            return true;
    }
    
    return false;
}

bool DebrisFallingTrigger::IsActive()
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
            
        if (unit->GetEntry() == NPC_TARGET_TRIGGER)
        {
            // Check for debris visual effect
            if (unit->HasAura(SPELL_DEBRIS_VISUAL))
            {
                if (bot->GetDistance(unit) < 10.0f)
                    return true;
            }
        }
    }
    
    return false;
}

bool CaveInTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;
        
    // Phase 2 at 30% health
    if (magtheridon->GetHealthPct() <= 30.0f)
    {
        // Check for camera shake (warning sign)
        if (bot->HasAura(SPELL_CAMERA_SHAKE))
            return true;
            
        // Or if collapse is happening
        GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
        for (auto& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->HasAura(SPELL_COLLAPSE_DAMAGE))
                return true;
        }
    }
    
    return false;
}

bool BlazeNearbyTrigger::IsActive()
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
            
        // Blaze ground effect (trigger NPC)
        if (unit->GetEntry() == 17653 || unit->HasAura(SPELL_BLAZE_SUMMON))
        {
            if (bot->GetDistance(unit) < 8.0f)
                return true;
        }
    }
    
    return false;
}

bool CleaveDangerTrigger::IsActive()
{
    if (botAI->IsTank(bot))
        return false;
        
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || !magtheridon->IsAlive())
        return false;
        
    // Check if in melee range and near tank
    if (!bot->IsWithinMeleeRange(magtheridon))
        return false;
        
    Group* group = bot->GetGroup();
    if (!group)
        return false;
        
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || member == bot)
            continue;
            
        if (botAI->IsTank(member))
        {
            if (bot->GetDistance(member) < 8.0f)
                return true;
        }
    }
    
    return false;
}

bool HellfireWarderActiveTrigger::IsActive()
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
            
        if (unit->GetEntry() == NPC_HELLFIRE_WARDER)
        {
            if (unit->IsInCombat())
                return true;
        }
    }
    
    return false;
}

bool TankPositionAddsTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;
        
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (!currentTarget)
        return false;
        
    uint32 entry = currentTarget->GetEntry();
    if (entry == NPC_HELLFIRE_CHANNELER || entry == NPC_HELLFIRE_WARDER)
    {
        // Check if we need to move adds away
        float centerX = -18.70f;
        float centerY = 2.24f;
        
        if (currentTarget->GetDistance2d(centerX, centerY) < 20.0f)
            return true;
    }
    
    return false;
}

bool HealerPositionNeededTrigger::IsActive()
{
    if (!botAI->IsHealer(bot))
        return false;
        
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;
        
    // Too close to boss
    if (bot->GetDistance(magtheridon) < 20.0f)
        return true;
        
    // Check for dangerous ground effects
    GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
            
        // Near blaze or debris
        if ((unit->GetEntry() == 17653 || unit->HasAura(SPELL_BLAZE_SUMMON)) && 
            bot->GetDistance(unit) < 10.0f)
            return true;
    }
    
    return false;
}

bool BanishPhaseActiveTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;
        
    // Check if Magtheridon is banished
    if (magtheridon->HasAura(SPELL_SHADOW_CAGE_STUN))
        return true;
        
    return false;
}

bool InfernalNearbyTrigger::IsActive()
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
            
        // Burning Abyssal from channelers
        if (unit->GetEntry() == 17454)
        {
            if (bot->GetDistance(unit) < 15.0f)
                return true;
        }
    }
    
    return false;
}

bool ChannelerInterruptNeededTrigger::IsActive()
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
            
        if (unit->GetEntry() == NPC_HELLFIRE_CHANNELER)
        {
            // Check for interruptible spells
            if (unit->FindCurrentSpellBySpellId(SPELL_DARK_MENDING) ||
                unit->FindCurrentSpellBySpellId(SPELL_SHADOW_BOLT_VOLLEY))
            {
                // Check if we can interrupt
                if (bot->GetDistance(unit) <= 30.0f)
                {
                    // Check class abilities
                    if (bot->getClass() == CLASS_ROGUE ||
                        bot->getClass() == CLASS_WARRIOR ||
                        bot->getClass() == CLASS_MAGE ||
                        bot->getClass() == CLASS_WARLOCK ||
                        bot->getClass() == CLASS_SHAMAN)
                    {
                        return true;
                    }
                }
            }
        }
    }
    
    return false;
}

bool MagtheridonPhaseTransitionTrigger::IsActive()
{
    static uint8 lastPhase = 255;
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    
    if (!magtheridon)
    {
        lastPhase = 255;
        return false;
    }
    
    uint8 currentPhase = 0;
    
    // Determine current phase
    if (magtheridon->HasAura(SPELL_SHADOW_CAGE))
    {
        currentPhase = 0; // Channelers phase
    }
    else if (magtheridon->GetHealthPct() <= 30.0f)
    {
        currentPhase = 2; // Cave in phase
    }
    else
    {
        currentPhase = 1; // Released phase
    }
    
    if (currentPhase != lastPhase)
    {
        lastPhase = currentPhase;
        return true;
    }
    
    return false;
}

bool ShadowBoltVolleyCastTrigger::IsActive()
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
            
        if (unit->GetEntry() == NPC_HELLFIRE_CHANNELER)
        {
            if (unit->FindCurrentSpellBySpellId(SPELL_SHADOW_BOLT_VOLLEY))
            {
                if (bot->GetDistance(unit) <= 30.0f)
                    return true;
            }
        }
    }
    
    return false;
}

bool MindExhaustionCheckTrigger::IsActive()
{
    // Check if we have exhaustion and shouldn't click cubes
    if (bot->HasAura(SPELL_MIND_EXHAUSTION))
    {
        Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
        if (magtheridon && magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA))
        {
            // We're exhausted during blast nova - bad situation
            return true;
        }
    }
    
    return false;
}

bool MagtheridonReleasedTrigger::IsActive()
{
    static bool wasReleased = false;
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    
    if (!magtheridon)
    {
        wasReleased = false;
        return false;
    }
    
    bool isReleased = !magtheridon->HasAura(SPELL_SHADOW_CAGE);
    
    if (isReleased && !wasReleased)
    {
        wasReleased = true;
        return true;
    }
    
    wasReleased = isReleased;
    return false;
}

bool MagtheridonLowHealthTrigger::IsActive()
{
    static bool wasLowHealth = false;
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    
    if (!magtheridon)
    {
        wasLowHealth = false;
        return false;
    }
    
    bool isLowHealth = magtheridon->GetHealthPct() <= 30.0f;
    
    if (isLowHealth && !wasLowHealth)
    {
        wasLowHealth = true;
        return true;
    }
    
    wasLowHealth = isLowHealth;
    return false;
}