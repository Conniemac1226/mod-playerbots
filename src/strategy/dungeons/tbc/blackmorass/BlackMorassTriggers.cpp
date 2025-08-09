#include "BlackMorassTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Player.h"
#include "Unit.h"
#include "Creature.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"

bool AeonusCleaveNearbyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Only trigger for non-tanks
    if (botAI->IsTank(bot))
        return false;

    // Check if Aeonus is present and bot is in cleave danger
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_AEONUS)
        {
            // Pattern from Karazhan: check if in front arc and close (cleave danger)
            if (bot->GetDistance(unit) < 10.0f && unit->HasInArc(M_PI / 2, bot))
                return true;
        }
    }
    
    return false;
}

bool AeonusEngagedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Aeonus is engaged in combat
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_AEONUS && unit->IsInCombat())
            return true;
    }
    
    return false;
}

bool ChronoLordDejaEngagedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Chrono Lord Deja is engaged in combat
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_CHRONO_LORD_DEJA && unit->IsInCombat())
            return true;
    }
    
    return false;
}

bool TemporusEngagedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Temporus is engaged in combat
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_TEMPORUS && unit->IsInCombat())
            return true;
    }
    
    return false;
}

bool TimeLapseNearbyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Chrono Lord Deja is present and bot is in AoE range
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_CHRONO_LORD_DEJA)
        {
            // Check if bot is in Time Lapse AoE range
            if (bot->GetDistance(unit) < 15.0f && unit->IsInCombat())
                return true;
        }
    }
    
    return false;
}

bool ArcaneDischargeNearbyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Chrono Lord Deja is present and bot is in AoE range
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_CHRONO_LORD_DEJA)
        {
            // Check if bot is in Arcane Discharge AoE range
            if (bot->GetDistance(unit) < 20.0f && unit->IsInCombat())
                return true;
        }
    }
    
    return false;
}

bool WingBuffetNearbyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Temporus is present and positioning is suboptimal
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_TEMPORUS && unit->IsInCombat())
        {
            float distance = bot->GetDistance(unit);
            // Trigger when positioning is suboptimal for wing buffet
            if (distance > 5.0f && distance < 15.0f)
                return true;
        }
    }
    
    return false;
}

// RESEARCHED: boss_temporus.cpp:54 - HASTEN buff needs dispel
bool TemporusHastenActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Only trigger for classes that can dispel
    if (bot->getClass() != CLASS_PRIEST && 
        bot->getClass() != CLASS_SHAMAN &&
        bot->getClass() != CLASS_WARLOCK)
        return false;

    // Check if Temporus has Hasten buff
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_TEMPORUS && unit->IsInCombat())
        {
            if (unit->HasAura(SPELL_HASTEN))
                return true;
        }
    }
    
    return false;
}

// RESEARCHED: boss_chrono_lord_deja.cpp:58 - ARCANE_BLAST needs interrupt  
bool DejaArcaneBlastCastingTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Only trigger for classes with interrupts
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

    // Check if Chrono Lord Deja is casting Arcane Blast
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 30.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 30.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_CHRONO_LORD_DEJA && unit->IsInCombat())
        {
            if (unit->HasUnitState(UNIT_STATE_CASTING) && 
                unit->FindCurrentSpellBySpellId(SPELL_ARCANE_BLAST))
                return true;
        }
    }
    
    return false;
}

// RESEARCHED: boss_aeonus.cpp:86 - ENRAGE increases damage
bool AeonusEnragedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Aeonus has Enrage buff
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_AEONUS && unit->IsInCombat())
        {
            if (unit->HasAura(SPELL_ENRAGE))
                return true;
        }
    }
    
    return false;
}