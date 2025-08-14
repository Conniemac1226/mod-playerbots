#include "Playerbots.h"
#include "ArcatrazActions.h"
#include "ArcatrazStrategy.h"
#include "AttackersValue.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Per-bot state management for Dalliah whirlwind avoidance (matches successful Drake pattern)
static std::map<ObjectGuid, uint32> g_dalliah_lastMoveTime;
static std::map<ObjectGuid, bool> g_dalliah_inSafePosition;

bool AttackMellicharAddsAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Warden Mellichar spawns adds in waves while immune to damage
    // Bots must prioritize adds over the immune warden
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* priorityAdd = nullptr;
    float closestDistance = 50.0f;
    
    // Priority order: Phase 1 > Millhouse > Other adds
    const uint32 mellicharAdds[] = {
        NPC_TRICKSTER, NPC_PH_HUNTER, NPC_MILLHOUSE, 
        NPC_AKKIRIS, NPC_SULFURON, NPC_TW_DRAK, NPC_BL_DRAK
    };
    
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        // Check if this is one of Mellichar's adds
        for (uint32 addId : mellicharAdds)
        {
            if (unit->GetEntry() == addId && AttackersValue::IsValidTarget(unit, bot))
            {
                float distance = bot->GetDistance(unit);
                
                // Special priority for Millhouse (helpful NPC, protect him)
                if (addId == NPC_MILLHOUSE)
                {
                    // Don't attack Millhouse - he's friendly
                    continue;
                }
                
                if (distance < closestDistance)
                {
                    priorityAdd = unit;
                    closestDistance = distance;
                }
                break;
            }
        }
    }

    if (priorityAdd)
    {
        // Switch target to priority add
        return Attack(priorityAdd);
    }
    
    return false;
}

bool AttackMellicharAddsAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    // Check if we're fighting Warden Mellichar (who is immune) OR Harbinger Skyriss
    Unit* warden = AI_VALUE2(Unit*, "find target", "warden mellichar");
    Unit* skyriss = AI_VALUE2(Unit*, "find target", "harbinger skyriss");
    
    if ((!warden || !warden->IsInCombat()) && (!skyriss || !skyriss->IsInCombat()))
        return false;
        
    // RESEARCHED: arcatraz.cpp - Warden releases hostile adds in sequence
    // NOTE: Millhouse (NPC_MILLHOUSE) is NOT included - he becomes friendly ally!
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    const uint32 mellicharAdds[] = {
        NPC_TRICKSTER, NPC_PH_HUNTER,      // Wave 1: Random
        NPC_AKKIRIS, NPC_SULFURON,         // Wave 3: Random  
        NPC_TW_DRAK, NPC_BL_DRAK,          // Wave 4: Random
        NPC_HARBINGER_SKYRISS               // Wave 5: Final boss
    };
    
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        for (uint32 addId : mellicharAdds)
        {
            if (unit->GetEntry() == addId)
                return true;
        }
    }
    
    return false;
}

bool AvoidVoidZoneAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "zereketh the unbound");
    if (!boss)
    {
        return false;
    }
    
    // RESEARCHED: Void Zone - boss_zereketh_the_unbound.cpp:55-57
    // Void zones persist and deal damage - must evacuate immediately!
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit)
        {
            continue;
        }
        
        // Void Zone is typically a trigger/area effect (check by spell effect)
        if (unit->GetEntry() == 21101 || unit->GetName() == "Void Zone") // Void Zone trigger
        {
            float distance = bot->GetExactDist2d(unit);
            if (distance < 12.0f) // Increased safety radius
            {
                // EMERGENCY: Move away from void zone
                float angle = bot->GetAngle(unit) + M_PI;
                float moveDistance = 15.0f - distance;
                float x = bot->GetPositionX() + cos(angle) * moveDistance;
                float y = bot->GetPositionY() + sin(angle) * moveDistance;
                float z = bot->GetPositionZ();
                return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                            MovementPriority::MOVEMENT_FORCED);
            }
        }
    }
    
    return false;
}

bool AvoidShadowNovaAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "zereketh the unbound");
    if (!boss)
    {
        return false;
    }
    
    // RESEARCHED: Shadow Nova - boss_zereketh_the_unbound.cpp:59-65
    // AoE centered on boss - evacuate immediately when cast starts!
    if (boss->FindCurrentSpellBySpellId(SPELL_SHADOW_NOVA))
    {
        float safeDistance = 25.0f; // Increased safety margin
        float currentDist = bot->GetExactDist2d(boss);
        
        if (currentDist < safeDistance)
        {
            // EMERGENCY: Move to safe range
            float angle = bot->GetAngle(boss) + M_PI;
            float moveDistance = safeDistance - currentDist + 5.0f;
            float x = bot->GetPositionX() + cos(angle) * moveDistance;
            float y = bot->GetPositionY() + sin(angle) * moveDistance;
            float z = bot->GetPositionZ();
            
            // Stop casting and move
            bot->InterruptNonMeleeSpells(true);
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                        MovementPriority::MOVEMENT_FORCED);
        }
    }
    
    return false;
}

bool SeedOfCorruptionDispelAction::Execute(Event event)
{
    // RESEARCHED: Seed of Corruption - boss_zereketh_the_unbound.cpp:67-69
    // Explodes on expiry or dispel - spread out if afflicted!
    if (bot->HasAura(SPELL_SEED_OF_CORRUPTION))
    {
        const GuidVector members = AI_VALUE(GuidVector, "group members");
        for (auto& member : members)
        {
            if (member == bot->GetGUID())
            {
                continue;
            }
            
            Unit* unit = botAI->GetUnit(member);
            if (!unit || !unit->IsAlive())
            {
                continue;
            }
            
            float distance = bot->GetExactDist2d(unit);
            if (distance < 15.0f) // Seed explodes in AoE
            {
                // EMERGENCY: Spread from allies
                float angle = bot->GetAngle(unit) + M_PI;
                float moveDistance = 20.0f - distance;
                float x = bot->GetPositionX() + cos(angle) * moveDistance;
                float y = bot->GetPositionY() + sin(angle) * moveDistance;
                float z = bot->GetPositionZ();
                return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                            MovementPriority::MOVEMENT_FORCED);
            }
        }
    }
    
    return false;
}

bool DalliahWhirlwindAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "dalliah the doomsayer");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // Per-bot state management (RESEARCHED: Successful Drake pattern)
    uint32 currentTime = getMSTime();
    ObjectGuid botGuid = bot->GetGUID();
    
    // Check if Dalliah is casting or has whirlwind active  
    bool isWhirlwinding = false;
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_WHIRLWIND))
        isWhirlwinding = true;
    if (boss->HasAura(SPELL_WHIRLWIND))
        isWhirlwinding = true;
    
    // Check if we're already in a safe position during this whirlwind
    if (g_dalliah_inSafePosition[botGuid] && isWhirlwinding)
    {
        // Check if this is a new whirlwind phase (10+ seconds since last move)
        if ((currentTime - g_dalliah_lastMoveTime[botGuid]) > 10000)
        {
            g_dalliah_inSafePosition[botGuid] = false;
            g_dalliah_lastMoveTime[botGuid] = 0;
        }
        else
        {
            return false; // Already safe, don't move again
        }
    }
    
    // Reset state if whirlwind ended
    if (!isWhirlwinding)
    {
        g_dalliah_inSafePosition[botGuid] = false;
        g_dalliah_lastMoveTime[botGuid] = 0;
        return false;
    }
    
    float distance = bot->GetDistance(boss);
    if (distance < 12.0f) // Increased trigger range for earlier escape
    {
        // Calculate unique safe position for this bot (RESEARCHED: Drake successful pattern)
        float baseAngle = (botGuid.GetCounter() % 8) * (M_PI / 4.0f); // Distribute bots in 8 directions
        float angle = baseAngle + frand(-0.2f, 0.2f); // Add small random variation
        float safeDistance = 15.0f; // Increased from 12 to 15 yards for better safety
        
        float targetX = boss->GetPositionX() + cos(angle) * safeDistance;
        float targetY = boss->GetPositionY() + sin(angle) * safeDistance;
        float targetZ = boss->GetPositionZ();
        
        // EMERGENCY: Move to safe position using FORCED priority for critical escape
        bool result = MoveTo(bot->GetMapId(), targetX, targetY, targetZ, 
                            false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        
        if (result)
        {
            g_dalliah_lastMoveTime[botGuid] = currentTime;
            g_dalliah_inSafePosition[botGuid] = true;
        }
        
        return result;
    }
    
    return false;
}

bool DalliahHealInterruptAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "dalliah the doomsayer");
    if (!boss)
    {
        return false;
    }
    
    // RESEARCHED: Heal cast after Whirlwind - boss_dalliah_the_doomsayer.cpp:104-108
    // Boss heals herself 7 seconds after whirlwind - MUST interrupt!
    if (boss->FindCurrentSpellBySpellId(SPELL_HEAL))
    {
        // Try melee interrupt if in range
        if (bot->IsWithinMeleeRange(boss))
        {
            botAI->InterruptSpell();
            return true;
        }
        
        // Try ranged interrupts - SAFE PATTERN
        Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "interrupt");
        if (spellIdsValue)
        {
            std::list<uint32> spellIds = spellIdsValue->Get();
            for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
            {
                uint32 spellId = *it;
                if (botAI->CanCastSpell(spellId, boss, false))
                {
                    return botAI->CastSpell(spellId, boss);
                }
            }
        }
    }
    
    return false;
}

bool SoccothratesKnockAwayAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "wrath-scryer soccothrates");
    if (!boss)
    {
        return false;
    }
    
    if (boss->FindCurrentSpellBySpellId(SPELL_KNOCK_AWAY))
    {
        float safeDistance = 15.0f;
        float currentDist = bot->GetExactDist2d(boss);
        
        if (currentDist < safeDistance)
        {
            return MoveAway(boss, safeDistance - currentDist + 2.0f);
        }
    }
    
    return false;
}

bool SoccothratesChargeAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "wrath-scryer soccothrates");
    if (!boss)
    {
        return false;
    }
    
    if (boss->HasAura(SPELL_FELFIRE) || boss->FindCurrentSpellBySpellId(SPELL_CHARGE))
    {
        const GuidVector members = AI_VALUE(GuidVector, "group members");
        for (auto& member : members)
        {
            if (member == bot->GetGUID())
            {
                continue;
            }
            
            Unit* unit = botAI->GetUnit(member);
            if (!unit)
            {
                continue;
            }
            
            float distance = bot->GetExactDist2d(unit);
            if (distance < 8.0f)
            {
                return MoveAway(unit, 10.0f - distance);
            }
        }
    }
    
    return false;
}

bool AvoidFelfireGroundAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "wrath-scryer soccothrates");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // RESEARCHED: NPC ID 20978 "Wrath-Scryer's Felfire" - persistent ground fire effects
    // Similar to Void Zone avoidance but for Soccothrates' Felfire ground effects
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* closestFelfire = nullptr;
    float closestDistance = 50.0f;
    
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit)
        {
            continue;
        }
        
        if (unit->GetEntry() == NPC_FELFIRE_GROUND && unit->IsAlive())
        {
            float distance = bot->GetExactDist2d(unit);
            if (distance < closestDistance)
            {
                closestFelfire = unit;
                closestDistance = distance;
            }
        }
    }
    
    if (closestFelfire && closestDistance < 10.0f) // Move if too close to any Felfire
    {
        // EMERGENCY: Move away from Felfire ground effect (similar to Void Zone pattern)
        float angle = bot->GetAngle(closestFelfire) + M_PI; // Opposite direction
        float escapeDistance = 12.0f - closestDistance; // Move to 12+ yards for safety
        float x = bot->GetPositionX() + cos(angle) * escapeDistance;
        float y = bot->GetPositionY() + sin(angle) * escapeDistance;
        float z = bot->GetPositionZ();
        
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                    MovementPriority::MOVEMENT_FORCED);
    }
    
    return false;
}

bool SkyrissIllusionAction::isUseful()
{
    return !botAI->IsHeal(bot);
}

bool SkyrissIllusionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "harbinger skyriss");
    if (!boss || !boss->IsInCombat())
    {
        return false;
    }
    
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit)
        {
            continue;
        }
        
        if (unit->GetEntry() == NPC_HARBINGER_ILLUSION && unit->IsAlive())
        {
            if (currentTarget != unit)
            {
                return Attack(unit);
            }
            return false;
        }
    }
    
    return false;
}

bool SkyrissFearAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "harbinger skyriss");
    if (!boss)
    {
        return false;
    }
    
    // RESEARCHED: Fear - boss_harbinger_skyriss.cpp:86-92
    // Boss casts Fear on random non-tank target every 25 seconds
    // If feared, try to break it or spread to prevent chain fears
    if (bot->HasAura(SPELL_FEAR))
    {
        // Try to use fear break abilities (PvP trinket, etc)
        Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "escape");
        if (spellIdsValue)
        {
            std::list<uint32> spellIds = spellIdsValue->Get();
            for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
            {
                uint32 spellId = *it;
                if (botAI->CanCastSpell(spellId, bot, false))
                {
                    return botAI->CastSpell(spellId, bot);
                }
            }
        }
    }
    // Detect fear cast and spread to minimize chain fears
    else if (boss->FindCurrentSpellBySpellId(SPELL_FEAR))
    {
        // Spread from other players to avoid chain fear
        const GuidVector members = AI_VALUE(GuidVector, "group members");
        for (auto& member : members)
        {
            if (member == bot->GetGUID())
                continue;
            
            Unit* unit = botAI->GetUnit(member);
            if (!unit || !unit->IsAlive())
                continue;
            
            float distance = bot->GetExactDist2d(unit);
            if (distance < 8.0f) // Fear can chain to nearby targets
            {
                // Move away from allies to prevent chain fear
                float angle = bot->GetAngle(unit) + M_PI;
                float x = bot->GetPositionX() + cos(angle) * 10.0f;
                float y = bot->GetPositionY() + sin(angle) * 10.0f;
                float z = bot->GetPositionZ();
                return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                            MovementPriority::MOVEMENT_FORCED);
            }
        }
    }
    
    return false;
}

bool SkyrissDominationAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "harbinger skyriss");
    if (!boss)
    {
        return false;
    }
    
    // RESEARCHED: Domination - boss_harbinger_skyriss.cpp:94-99
    // Boss casts Domination (mind control) on random target every 30 seconds
    // Need to handle controlled ally as hostile temporarily
    if (bot->HasAura(SPELL_DOMINATION))
    {
        // Bot is mind controlled - can't do anything
        // Just wait for it to expire or be dispelled
        return false;
    }
    
    // Check if any ally is dominated and keep distance
    const GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        if (member == bot->GetGUID())
            continue;
        
        Unit* unit = botAI->GetUnit(member);
        if (!unit || !unit->IsAlive())
            continue;
        
        // Check if this ally is mind controlled
        if (unit->HasAura(SPELL_DOMINATION))
        {
            float distance = bot->GetExactDist2d(unit);
            if (distance < 15.0f) // Stay away from dominated allies
            {
                // Move away from mind controlled ally
                float angle = bot->GetAngle(unit) + M_PI;
                float x = bot->GetPositionX() + cos(angle) * 20.0f;
                float y = bot->GetPositionY() + sin(angle) * 20.0f;
                float z = bot->GetPositionZ();
                return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                            MovementPriority::MOVEMENT_FORCED);
            }
        }
    }
    
    return false;
}