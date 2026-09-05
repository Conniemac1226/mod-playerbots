#include "Playerbots.h"
#include "BotanicaActions.h"
#include "BotanicaStrategy.h"

bool SarannisResonanceDispelAction::Execute(Event event)
{
    if (bot->HasAura(SPELL_ARCANE_RESONANCE))
    {
        // This debuff needs to be handled by dispel/cleanse abilities
        // The action triggers but actual dispel is handled by class-specific logic
        return true;
    }
    
    return false;
}

bool SarannisReinforcementsAction::isUseful()
{
    return !botAI->IsHeal(bot);
}

bool SarannisReinforcementsAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "commander sarannis");
    if (!boss || !boss->IsInCombat())
    {
        return false;
    }
    
    float distance = bot->GetExactDist2d(boss);
    if (distance > 40.0f)
    {
        // Pull adds back toward the boss if we are too far out
        return MoveTo(boss->GetMapId(), boss->GetPositionX(), boss->GetPositionY(), boss->GetPositionZ(),
                      false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
    }
    
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit)
            continue;
        
        if (unit->GetEntry() == NPC_BLOODWARDER_MENDER)
        {
            return Attack(unit);
        }
    }
    
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit)
            continue;
        
        if (unit->GetEntry() == NPC_BLOODWARDER_RESERVIST)
        {
            return Attack(unit);
        }
    }
    
    return false;
}


bool FreywinnFrayerPriorityAction::isUseful()
{
    return !botAI->IsHeal(bot);
}

bool FreywinnFrayerPriorityAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "high botanist freywinn");
    if (!boss || !boss->IsInCombat())
    {
        return false;
    }
    
    if (!boss->HasAura(SPELL_TREE_FORM))
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
        
        if (unit->GetEntry() == NPC_FRAYER && unit->IsAlive())
        {
            return Attack(unit);
        }
    }
    
    return false;
}

bool FreywinnTranquilityAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "high botanist freywinn");
    if (!boss)
    {
        return false;
    }
    
    // RESEARCHED: Tranquility - boss_high_botanist_freywinn.cpp:77-78
    // Boss casts Tranquility in tree form - MUST interrupt or kill frayers!
    if (boss->HasAura(SPELL_TREE_FORM) && boss->FindCurrentSpellBySpellId(SPELL_TRANQUILITY))
    {
        // Check if frayers are still alive - they must die first
        const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        for (auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (!unit)
            {
                continue;
            }
            
            if (unit->GetEntry() == NPC_FRAYER && unit->IsAlive())
            {
                // Frayers still alive - can't interrupt while they exist
                return Attack(unit);
            }
        }
        
        // All frayers dead - try to interrupt Tranquility
        // Try melee interrupt if in range
        if (bot->IsWithinMeleeRange(boss))
        {
            botAI->GetBot()->InterruptNonMeleeSpells(true);
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

bool LajAllergicReactionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "laj");
    if (!boss)
    {
        return false;
    }
    
    if (bot->HasAura(SPELL_ALLERGIC_REACTION))
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
            if (distance < 10.0f)
            {
                return MoveAway(unit, 12.0f - distance);
            }
        }
    }
    
    return false;
}

bool LajTeleportPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "laj");
    if (!boss)
    {
        return false;
    }
    
    if (boss->FindCurrentSpellBySpellId(SPELL_TELEPORT_SELF))
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

bool ThorngrinSacrificeAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "thorngrin the tender");
    if (!boss)
    {
        return false;
    }
    
    // RESEARCHED: Sacrifice - boss_thorngrin_the_tender.cpp:76-82
    // Boss sacrifices random target, they need massive healing
    if (botAI->IsHeal(bot))
    {
        // Find sacrificed target and prioritize healing them
        const GuidVector members = AI_VALUE(GuidVector, "group members");
        for (auto& member : members)
        {
            Unit* unit = botAI->GetUnit(member);
            if (!unit || !unit->IsAlive())
                continue;
            
            if (unit->HasAura(BOT_SPELL_SACRIFICE))
            {
                // Emergency heal on sacrificed target
                Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "heal");
                if (spellIdsValue)
                {
                    std::list<uint32> spellIds = spellIdsValue->Get();
                    for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
                    {
                        uint32 spellId = *it;
                        if (botAI->CanCastSpell(spellId, unit, false))
                        {
                            return botAI->CastSpell(spellId, unit);
                        }
                    }
                }
            }
        }
    }
    
    return false;
}

bool ThorngrinHellfireAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "thorngrin the tender");
    if (!boss)
    {
        return false;
    }

    float safeDistance = 15.0f; // Hellfire radius per spell data
    float currentDist = bot->GetExactDist2d(boss);

    if (currentDist < safeDistance)
    {
        float moveDistance = safeDistance - currentDist + 4.0f;

        bot->InterruptNonMeleeSpells(true);
        if (MoveAway(boss, moveDistance))
            return true;

        float baseAngle = boss->GetAngle(bot);
        for (float delta = 0.0f; delta <= M_PI / 2; delta += M_PI / 8)
        {
            for (int sign = -1; sign <= 1; sign += 2)
            {
                float angle = baseAngle + delta * sign;
                if (delta == 0.0f && sign == -1)
                    sign = 1; // ensure base angle only processed once

                float dx = bot->GetPositionX() + cos(angle) * moveDistance;
                float dy = bot->GetPositionY() + sin(angle) * moveDistance;
                float dz = bot->GetPositionZ();
                bool exact = bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                                            bot->GetPositionZ(), dx, dy, dz);
                if (!exact)
                {
                    dx = bot->GetPositionX() + cos(angle) * moveDistance;
                    dy = bot->GetPositionY() + sin(angle) * moveDistance;
                    dz = bot->GetPositionZ();
                }

                if (MoveTo(bot->GetMapId(), dx, dy, dz, false, false, true, exact,
                          MovementPriority::MOVEMENT_NORMAL))
                    return true;
            }
        }
    }

    return false;
}




bool ThorngrinEnrageAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "thorngrin the tender");
    if (!boss)
    {
        return false;
    }
    
    // RESEARCHED: Enrage - boss_thorngrin_the_tender.cpp:92
    // Boss gains enrage buff every 30 seconds, increasing damage
    // Dispel if possible, otherwise healers need to be more careful
    if (boss->HasAura(BOT_SPELL_ENRAGE))
    {
        // Try to dispel enrage if we have magic dispel abilities
        Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "dispel magic");
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
        
        // If we can't dispel and we're a healer, prioritize low health targets
        if (botAI->IsHeal(bot))
        {
            // Enrage means more damage - find targets that need emergency healing
            const GuidVector members = AI_VALUE(GuidVector, "group members");
            Unit* lowestHealthTarget = nullptr;
            float lowestHealthPct = 100.0f;
            
            for (auto& member : members)
            {
                Unit* unit = botAI->GetUnit(member);
                if (!unit || !unit->IsAlive())
                    continue;
                
                float healthPct = (float)unit->GetHealth() / (float)unit->GetMaxHealth() * 100.0f;
                if (healthPct < lowestHealthPct && healthPct < 80.0f) // Focus on targets below 80%
                {
                    lowestHealthTarget = unit;
                    lowestHealthPct = healthPct;
                }
            }
            
            if (lowestHealthTarget)
            {
                Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "heal");
                if (spellIdsValue)
                {
                    std::list<uint32> spellIds = spellIdsValue->Get();
                    for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
                    {
                        uint32 spellId = *it;
                        if (botAI->CanCastSpell(spellId, lowestHealthTarget, false))
                        {
                            return botAI->CastSpell(spellId, lowestHealthTarget);
                        }
                    }
                }
            }
        }
    }
    
    return false;
}

bool WarpSplinterWarStompAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warp splinter");
    if (!boss)
    {
        return false;
    }
    
    if (boss->FindCurrentSpellBySpellId(BOT_SPELL_WAR_STOMP))
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

bool WarpSplinterArcaneVolleyAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warp splinter");
    if (!boss)
    {
        return false;
    }
    
    // RESEARCHED: Arcane Volley is a channeled spell that needs interrupting
    if (boss->FindCurrentSpellBySpellId(SPELL_ARCANE_VOLLEY))
    {
        // Try melee interrupt if in range
        if (bot->IsWithinMeleeRange(boss))
        {
            botAI->GetBot()->InterruptNonMeleeSpells(true);
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
