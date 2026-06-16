#include "BlackMorassActions.h"
#include "BlackMorassTriggers.h"
#include "AiObjectContext.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

namespace
{
Unit* FindPortalAdd(PlayerbotAI* botAI, Player* bot)
{
    Unit* medivh = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "medivh")->Get();
    Unit* target = nullptr;
    float bestScore = 100.0f;

    GuidVector const npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
    for (ObjectGuid const& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive() || !IsBlackMorassPortalAdd(unit->GetEntry()))
            continue;

        float score = bot->GetDistance(unit);
        if (medivh && unit->GetDistance(medivh) < 30.0f)
            score -= 20.0f;

        if (score < bestScore)
        {
            bestScore = score;
            target = unit;
        }
    }

    return target;
}

bool ShouldDismountForPortalAdd(PlayerbotAI* botAI, Unit* target)
{
    Player* bot = botAI->GetBot();
    if (!bot || !bot->IsMounted() || !target)
        return false;

    Player* master = botAI->GetMaster();
    if (master && !master->IsMounted())
        return true;

    return bot->GetExactDist2d(target) <= 20.0f;
}
}

// ========== PORTAL/ADD MANAGEMENT ==========

bool AttackPortalAddAction::Execute(Event /*event*/)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* target = FindPortalAdd(botAI, bot);
    if (!target)
        return false;

    if (ShouldDismountForPortalAdd(botAI, target))
    {
        bot->Dismount();
        return true;
    }

    // Keep the portal-add action active while the bot is still riding in.
    if (bot->IsMounted())
        return true;

    return Attack(target);
}

bool AttackPortalAddAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    return FindPortalAdd(botAI, bot);
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
                     false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }

    return false;
}

bool ProtectMedivhAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* medivh = AI_VALUE2(Unit*, "find target", "medivh");
    if (!medivh || !medivh->IsAlive())
        return false;

    return bot->GetDistance(medivh) > 20.0f;
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
                     MovementPriority::MOVEMENT_FORCED);
    }

    return false;
}

bool AeonusAvoidCleaveAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || botAI->IsTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "aeonus");
    return boss && boss->IsAlive() && boss->IsInCombat() && bot->GetDistance(boss) < 10.0f &&
           boss->HasInArc(M_PI / 2, bot);
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
                             MovementPriority::MOVEMENT_FORCED);
            }
        }
    }

    return false;
}

bool AeonusPositionAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI->IsTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "aeonus");
    return boss && boss->IsAlive() && boss->IsInCombat();
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
            // Move away from Sand Breath cone
            float angle = bot->GetAngle(boss) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 15.0f;
            float y = bot->GetPositionY() + sin(angle) * 15.0f;
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                        MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool AeonusSandBreathAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "aeonus");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_SAND_BREATH) &&
           boss->HasInArc(M_PI / 4, bot);
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
        const GuidVector members = AI_VALUE(GuidVector, "group members");
        for (auto& member : members)
        {
            Unit* ally = botAI->GetUnit(member);
            if (ally && ally != bot && ally->IsAlive())
            {
                float distance = bot->GetDistance(ally);
                if (distance < 10.0f)
                {
                    // Spread out from allies
                    float angle = bot->GetAngle(ally) + M_PI;
                    float x = bot->GetPositionX() + cos(angle) * 15.0f;
                    float y = bot->GetPositionY() + sin(angle) * 15.0f;
                    float z = bot->GetPositionZ();
                    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                                MovementPriority::MOVEMENT_FORCED);
                }
            }
        }
    }

    return false;
}

bool AvoidTimeLapseAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "chrono lord deja");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat() || !boss->FindCurrentSpellBySpellId(SPELL_TIME_LAPSE))
        return false;

    GuidVector const members = AI_VALUE(GuidVector, "group members");
    for (ObjectGuid const& member : members)
    {
        Unit* ally = botAI->GetUnit(member);
        if (ally && ally != bot && ally->IsAlive() && bot->GetDistance(ally) < 10.0f)
            return true;
    }

    return false;
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
            // Move away from Arcane Discharge
            float angle = bot->GetAngle(boss) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 25.0f;
            float y = bot->GetPositionY() + sin(angle) * 25.0f;
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                        MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool AvoidArcaneDischargeAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "chrono lord deja");
    return boss && boss->IsAlive() && boss->IsInCombat() && boss->FindCurrentSpellBySpellId(SPELL_ARCANE_DISCHARGE) &&
           bot->GetDistance(boss) < 20.0f;
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
        const GuidVector members = AI_VALUE(GuidVector, "group members");
        for (auto& member : members)
        {
            Unit* ally = botAI->GetUnit(member);
            if (ally && ally != bot && ally->IsAlive())
            {
                float distance = bot->GetDistance(ally);
                if (distance < 15.0f)
                {
                    // Move away from raid when attracted
                    float angle = bot->GetAngle(ally) + M_PI;
                    float x = bot->GetPositionX() + cos(angle) * 20.0f;
                    float y = bot->GetPositionY() + sin(angle) * 20.0f;
                    float z = bot->GetPositionZ();
                    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                                MovementPriority::MOVEMENT_FORCED);
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
                         MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool AvoidWingBuffetAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "temporus");
    return boss && boss->IsAlive() && boss->IsInCombat() && boss->FindCurrentSpellBySpellId(SPELL_WING_BUFFET) &&
           bot->GetDistance(boss) < 10.0f;
}

bool TemporusMortalWoundAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI->IsHeal(bot))
        return false;

    GuidVector const members = AI_VALUE(GuidVector, "group members");
    for (ObjectGuid const& member : members)
    {
        Unit* ally = botAI->GetUnit(member);
        if (ally && ally->IsAlive() && ally->HasAura(SPELL_MORTAL_WOUND) && ally->GetHealthPct() < 60.0f)
        {
            return botAI->CastSpell("heal", ally);
        }
    }

    return false;
}

bool TemporusMortalWoundAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI->IsHeal(bot))
        return false;

    GuidVector const members = AI_VALUE(GuidVector, "group members");
    for (ObjectGuid const& member : members)
    {
        Unit* ally = botAI->GetUnit(member);
        if (ally && ally->IsAlive() && ally->HasAura(SPELL_MORTAL_WOUND))
            return true;
    }

    return false;
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
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "temporus");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat() || !boss->HasAura(SPELL_REFLECT))
        return false;

    return bot->IsNonMeleeSpellCast(false);
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
