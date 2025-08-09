#include "BlackMorassActions.h"
#include "BlackMorassTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// Per-bot state management for boss mechanics
std::map<ObjectGuid, uint32> g_aeonus_lastTimeStopTime;
std::map<ObjectGuid, bool> g_aeonus_timeStopActive;

// ========== PORTAL/ADD MANAGEMENT ==========

bool AttackPortalAddAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Priority order for adds (most dangerous first)
    const uint32 addPriority[] = {
        NPC_RIFT_LORD, NPC_RIFT_LORD_2,           // Highest priority
        NPC_RIFT_KEEPER_WARLOCK, NPC_RIFT_KEEPER_MAGE,
        NPC_INFINITE_EXECUTIONER, NPC_INFINITE_VANQUISHER,
        NPC_INFINITE_CHRONOMANCER, NPC_INFINITE_ASSASSIN,
        NPC_INFINITE_WHELP                        // Lowest priority
    };

    // Use proven WotLK pattern
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* priorityTarget = nullptr;
    float closestDistance = 100.0f;

    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        for (uint32 addId : addPriority)
        {
            if (unit->GetEntry() == addId)
            {
                float distance = bot->GetDistance(unit);
                
                // Prioritize adds moving toward Medivh
                Unit* medivh = AI_VALUE2(Unit*, "find target", "medivh");
                if (medivh && unit->GetDistance(medivh) < 30.0f)
                {
                    distance -= 20.0f; // Higher priority for adds near Medivh
                }
                
                if (distance < closestDistance)
                {
                    priorityTarget = unit;
                    closestDistance = distance;
                }
                break;
            }
        }
    }

    if (priorityTarget)
    {
        return Attack(priorityTarget);
    }

    return false;
}

bool AttackPortalAddAction::isUseful()
{
    return AI_VALUE(bool, "portal add active");
}

bool ProtectMedivhAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* medivh = AI_VALUE2(Unit*, "find target", "medivh");
    if (!medivh)
        return false;

    float distance = bot->GetDistance(medivh);
    
    // Stay within 20 yards of Medivh
    if (distance > 20.0f)
    {
        Position medivhPos = medivh->GetPosition();
        float angle = medivh->GetAngle(bot);
        float moveDistance = 15.0f;
        
        float x = medivhPos.GetPositionX() + cos(angle) * moveDistance;
        float y = medivhPos.GetPositionY() + sin(angle) * moveDistance;
        
        return MoveTo(bot->GetMapId(), x, y, medivhPos.GetPositionZ(),
                     false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool ProtectMedivhAction::isUseful()
{
    return AI_VALUE(bool, "medivh needs protection");
}

// ========== AEONUS ACTIONS ==========

bool AeonusAvoidCleaveAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "aeonus");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if we're in front of the boss (cleave danger)
    if (boss->HasInArc(M_PI / 2, bot) && !botAI->IsTank(bot))
    {
        // Move to boss's side
        float angle = boss->GetOrientation() + (M_PI / 2);
        Position safePos = boss->GetPosition();
        safePos.m_positionX += cos(angle) * 8.0f;
        safePos.m_positionY += sin(angle) * 8.0f;
        
        return MoveTo(bot->GetMapId(), safePos.m_positionX, safePos.m_positionY, 
                     safePos.m_positionZ, false, false, false, true, 
                     MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool AeonusAvoidCleaveAction::isUseful()
{
    return AI_VALUE(bool, "aeonus cleave danger");
}

bool AeonusPositionAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI->IsTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "aeonus");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Tank should face boss away from group
    if (botAI->HasAggro(boss))
    {
        // Position boss facing away from Medivh/group
        Unit* medivh = AI_VALUE2(Unit*, "find target", "medivh");
        if (medivh)
        {
            float angle = medivh->GetAngle(boss) + M_PI;
            Position tankPos = boss->GetPosition();
            tankPos.m_positionX += cos(angle) * 5.0f;
            tankPos.m_positionY += sin(angle) * 5.0f;
            
            float distance = bot->GetDistance(tankPos);
            if (distance > 3.0f)
            {
                return MoveTo(bot->GetMapId(), tankPos.m_positionX, tankPos.m_positionY,
                             tankPos.m_positionZ, false, false, false, true,
                             MovementPriority::MOVEMENT_COMBAT);
            }
        }
    }

    return false;
}

bool AeonusPositionAction::isUseful()
{
    return AI_VALUE(bool, "aeonus engaged");
}

bool AeonusSandBreathAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "aeonus");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Sand Breath from boss_aeonus.cpp
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_SAND_BREATH))
    {
        if (boss->HasInArc(M_PI / 4, bot)) // Narrower cone
        {
            return FleePosition(boss->GetPosition(), 15.0f, 500U);
        }
    }

    return false;
}

bool AeonusSandBreathAction::isUseful()
{
    return AI_VALUE(bool, "sand breath danger");
}

bool AeonusTimeStopAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    uint32 currentTime = getMSTime();

    // Check if Time Stop is active
    if (bot->HasAura(SPELL_TIME_STOP))
    {
        g_aeonus_timeStopActive[botGuid] = true;
        g_aeonus_lastTimeStopTime[botGuid] = currentTime;
        
        // Stop all actions during Time Stop
        botAI->InterruptSpell();
        return true;
    }
    
    // Reset state after Time Stop ends
    if (g_aeonus_timeStopActive[botGuid])
    {
        if ((currentTime - g_aeonus_lastTimeStopTime[botGuid]) > 3000)
        {
            g_aeonus_timeStopActive[botGuid] = false;
        }
    }

    return false;
}

bool AeonusTimeStopAction::isUseful()
{
    return AI_VALUE(bool, "time stop active");
}

// Handle Aeonus Enrage - increased damage
bool AeonusEnrageAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "aeonus");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Enrage from boss_aeonus.cpp:86 - every 30s
    if (boss->HasAura(SPELL_ENRAGE))
    {
        // Healers need to increase healing
        if (botAI->IsHeal(bot))
        {
            // Focus heal on tank
            GuidVector members = AI_VALUE(GuidVector, "group members");
            for (auto& member : members)
            {
                Unit* ally = botAI->GetUnit(member);
                if (ally && ally->IsAlive() && ally->IsPlayer() && botAI->IsTank(ally->ToPlayer()))
                {
                    if (ally->GetHealthPct() < 80.0f)
                    {
                        return botAI->CastSpell("greater heal", ally);
                    }
                }
            }
        }
        
        // DPS should use cooldowns
        if (!botAI->IsTank(bot) && !botAI->IsHeal(bot))
        {
            return botAI->CastSpell("boost", bot);
        }
    }

    return false;
}

bool AeonusEnrageAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "aeonus");
    return boss && boss->HasAura(SPELL_ENRAGE);
}

// ========== CHRONO LORD DEJA ACTIONS ==========

bool AvoidTimeLapseAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "chrono lord deja");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Time Lapse from boss_chrono_lord_deja.cpp
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_TIME_LAPSE))
    {
        // Spread out to minimize teleport clustering
        GuidVector members = AI_VALUE(GuidVector, "group members");
        for (auto& member : members)
        {
            Unit* ally = botAI->GetUnit(member);
            if (ally && ally != bot && ally->IsAlive())
            {
                float distance = bot->GetDistance(ally);
                if (distance < 10.0f)
                {
                    return FleePosition(ally->GetPosition(), 15.0f, 500U);
                }
            }
        }
    }

    return false;
}

bool AvoidTimeLapseAction::isUseful()
{
    return AI_VALUE(bool, "time lapse danger");
}

bool AvoidArcaneDischargeAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "chrono lord deja");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Arcane Discharge from boss_chrono_lord_deja.cpp
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_ARCANE_DISCHARGE))
    {
        float distance = bot->GetDistance(boss);
        if (distance < 20.0f)
        {
            return FleePosition(boss->GetPosition(), 25.0f, 500U);
        }
    }

    return false;
}

bool AvoidArcaneDischargeAction::isUseful()
{
    return AI_VALUE(bool, "arcane discharge danger");
}

bool DejaAttractionAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Attraction from boss_chrono_lord_deja.cpp (Heroic)
    if (bot->HasAura(SPELL_ATTRACTION))
    {
        // Move away from raid to avoid damage
        GuidVector members = AI_VALUE(GuidVector, "group members");
        for (auto& member : members)
        {
            Unit* ally = botAI->GetUnit(member);
            if (ally && ally != bot && ally->IsAlive())
            {
                float distance = bot->GetDistance(ally);
                if (distance < 15.0f)
                {
                    return FleePosition(ally->GetPosition(), 20.0f, 500U);
                }
            }
        }
    }

    return false;
}

bool DejaAttractionAction::isUseful()
{
    Player* bot = botAI->GetBot();
    return bot && bot->HasAura(SPELL_ATTRACTION);
}

// ========== TEMPORUS ACTIONS ==========

bool AvoidWingBuffetAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "temporus");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Wing Buffet from boss_temporus.cpp
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_WING_BUFFET))
    {
        // Move to the side to avoid frontal knockback
        if (boss->HasInArc(M_PI / 3, bot))
        {
            float angle = boss->GetOrientation() + (M_PI / 2);
            Position safePos = boss->GetPosition();
            safePos.m_positionX += cos(angle) * 10.0f;
            safePos.m_positionY += sin(angle) * 10.0f;
            
            return MoveTo(bot->GetMapId(), safePos.m_positionX, safePos.m_positionY,
                         safePos.m_positionZ, false, false, false, true,
                         MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool AvoidWingBuffetAction::isUseful()
{
    return AI_VALUE(bool, "wing buffet danger");
}

bool TemporusMortalWoundAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Mortal Wound from boss_temporus.cpp
    if (bot->HasAura(SPELL_MORTAL_WOUND))
    {
        // Healers should prioritize healing targets with Mortal Wound
        if (botAI->IsHeal(bot))
        {
            GuidVector members = AI_VALUE(GuidVector, "group members");
            for (auto& member : members)
            {
                Unit* ally = botAI->GetUnit(member);
                if (ally && ally->IsAlive() && ally->HasAura(SPELL_MORTAL_WOUND))
                {
                    if (ally->GetHealthPct() < 60.0f)
                    {
                        return botAI->CastSpell("heal", ally);
                    }
                }
            }
        }
    }

    return false;
}

bool TemporusMortalWoundAction::isUseful()
{
    return AI_VALUE(bool, "mortal wound active");
}

bool TemporusReflectAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "temporus");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Spell Reflect from boss_temporus.cpp (Heroic)
    if (boss->HasAura(SPELL_REFLECT))
    {
        // Stop casting harmful spells
        if (bot->IsNonMeleeSpellCast(false))
        {
            botAI->InterruptSpell();
            return true;
        }
    }

    return false;
}

bool TemporusReflectAction::isUseful()
{
    return AI_VALUE(bool, "temporus reflect active");
}

// ========== TEMPORUS HASTEN DISPEL ==========

bool TemporusHastenDispelAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "temporus");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Hasten from boss_temporus.cpp:54 - cast on self every 20s
    if (boss->HasAura(SPELL_HASTEN))
    {
        // Priests can dispel magic, Shamans can purge
        if (bot->getClass() == CLASS_PRIEST)
        {
            return botAI->CastSpell("dispel magic", boss);
        }
        else if (bot->getClass() == CLASS_SHAMAN)
        {
            return botAI->CastSpell("purge", boss);
        }
        else if (bot->getClass() == CLASS_WARLOCK && bot->GetLevel() >= 70)
        {
            // Felhunter's Devour Magic (if pet is out)
            if (Pet* pet = bot->GetPet())
            {
                if (pet->GetEntry() == 417) // Felhunter
                {
                    return botAI->CastSpell("devour magic", boss);
                }
            }
        }
    }

    return false;
}

bool TemporusHastenDispelAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Only useful for classes that can dispel/purge
    if (bot->getClass() != CLASS_PRIEST && 
        bot->getClass() != CLASS_SHAMAN &&
        bot->getClass() != CLASS_WARLOCK)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "temporus");
    return boss && boss->HasAura(SPELL_HASTEN);
}

// ========== CHRONO LORD DEJA ARCANE BLAST INTERRUPT ==========

bool DejaArcaneBlastInterruptAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "chrono lord deja");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Arcane Blast from boss_chrono_lord_deja.cpp:58 - interruptible
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_ARCANE_BLAST))
    {
        // Check if we have an interrupt available
        uint32 interruptSpell = 0;
        
        switch(bot->getClass())
        {
            case CLASS_WARRIOR:
                interruptSpell = 6552; // Pummel
                break;
            case CLASS_ROGUE:
                interruptSpell = 1766; // Kick
                break;
            case CLASS_SHAMAN:
                interruptSpell = 8042; // Earth Shock
                break;
            case CLASS_MAGE:
                interruptSpell = 2139; // Counterspell
                break;
            case CLASS_PRIEST:
                if (bot->GetLevel() >= 70)
                    interruptSpell = 15487; // Silence (Shadow)
                break;
        }
        
        if (interruptSpell && !bot->HasSpellCooldown(interruptSpell))
        {
            return botAI->CastSpell(interruptSpell, boss);
        }
    }

    return false;
}

bool DejaArcaneBlastInterruptAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Only useful for classes with interrupts
    switch(bot->getClass())
    {
        case CLASS_WARRIOR:
        case CLASS_ROGUE:
        case CLASS_SHAMAN:
        case CLASS_MAGE:
        case CLASS_PRIEST:
            break;
        default:
            return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "chrono lord deja");
    return boss && boss->IsAlive() && boss->IsInCombat();
}