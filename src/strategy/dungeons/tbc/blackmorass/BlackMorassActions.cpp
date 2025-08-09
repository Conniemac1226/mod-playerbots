#include "BlackMorassActions.h"
#include "BlackMorassTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Player.h"
#include "Unit.h"
#include "Creature.h"
#include "SharedDefines.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"

bool AeonusAvoidCleaveAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find Aeonus
    Unit* aeonus = nullptr;
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_AEONUS)
        {
            aeonus = unit;
            break;
        }
    }

    if (!aeonus)
        return false;

    // Position behind Aeonus to avoid cleave (pattern from Karazhan Attumen)
    if (bot->GetDistance(aeonus) < 10.0f && aeonus->HasInArc(M_PI / 2, bot))
    {
        float angle = aeonus->GetOrientation() + M_PI;  // Move to back
        float x = aeonus->GetPositionX() + cos(angle) * 6.0f;
        float y = aeonus->GetPositionY() + sin(angle) * 6.0f;
        float z = aeonus->GetPositionZ();
        
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }
    
    return false;
}

bool AeonusAvoidCleaveAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Only useful for non-tanks
    if (botAI->IsTank(bot))
        return false;

    // Find Aeonus and check if in front arc (cleave danger zone)
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_AEONUS)
        {
            // Check if we're in cleave danger zone (close and in front)
            if (bot->GetDistance(unit) < 10.0f && unit->HasInArc(M_PI / 2, bot))
                return true;
        }
    }
    
    return false;
}

bool AeonusPositionAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find Aeonus
    Unit* aeonus = nullptr;
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_AEONUS)
        {
            aeonus = unit;
            break;
        }
    }

    if (!aeonus)
        return false;

    // Tanks should position boss facing away from group
    if (botAI->IsTank(bot))
    {
        Unit* myTarget = bot->GetVictim();
        if (myTarget && myTarget == aeonus)
        {
            // Position away from other group members
            float angle = aeonus->GetOrientation();
            float x = aeonus->GetPositionX() + cos(angle + M_PI) * 8.0f;
            float y = aeonus->GetPositionY() + sin(angle + M_PI) * 8.0f;
            float z = aeonus->GetPositionZ();
            
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }
    
    return false;
}

bool AeonusPositionAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Only useful for tanks
    if (!botAI->IsTank(bot))
        return false;

    // Check if tanking Aeonus
    Unit* victim = bot->GetVictim();
    return (victim && victim->GetEntry() == NPC_AEONUS);
}

bool AvoidTimeLapseAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find Chrono Lord Deja
    Unit* boss = nullptr;
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_CHRONO_LORD_DEJA)
        {
            boss = unit;
            break;
        }
    }

    if (!boss)
        return false;

    // Spread out from boss and other players for AoE avoidance
    float distance = bot->GetDistance(boss);
    if (distance < 15.0f)  // Time Lapse has wide range
    {
        // Move away from boss
        float angle = bot->GetAngle(boss) + M_PI;  // Opposite direction
        float x = bot->GetPositionX() + cos(angle) * 10.0f;
        float y = bot->GetPositionY() + sin(angle) * 10.0f;
        float z = bot->GetPositionZ();
        
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool AvoidTimeLapseAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Chrono Lord Deja is present and we're too close
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_CHRONO_LORD_DEJA)
        {
            // Check if we're in AoE range
            if (bot->GetDistance(unit) < 15.0f)
                return true;
        }
    }
    
    return false;
}

bool AvoidArcaneDischargeAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find Chrono Lord Deja
    Unit* boss = nullptr;
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_CHRONO_LORD_DEJA)
        {
            boss = unit;
            break;
        }
    }

    if (!boss)
        return false;

    // Spread out for AoE discharge avoidance  
    float distance = bot->GetDistance(boss);
    if (distance < 20.0f)  // Arcane Discharge has large AoE
    {
        // Move away from boss and spread from other players
        float angle = bot->GetAngle(boss) + M_PI + frand(-0.5f, 0.5f);  // Add randomization
        float x = bot->GetPositionX() + cos(angle) * 12.0f;
        float y = bot->GetPositionY() + sin(angle) * 12.0f;
        float z = bot->GetPositionZ();
        
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool AvoidArcaneDischargeAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Chrono Lord Deja is present and we need to spread
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_CHRONO_LORD_DEJA)
        {
            // Check if we're in AoE range
            if (bot->GetDistance(unit) < 20.0f)
                return true;
        }
    }
    
    return false;
}

bool AvoidWingBuffetAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find Temporus
    Unit* boss = nullptr;
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_TEMPORUS)
        {
            boss = unit;
            break;
        }
    }

    if (!boss)
        return false;

    // Get in position to minimize wing buffet knockback
    float distance = bot->GetDistance(boss);
    if (distance > 5.0f && distance < 15.0f)  // Wing buffet optimal positioning
    {
        if (botAI->IsTank(bot))
        {
            // Tanks stay close
            float angle = bot->GetAngle(boss);
            float x = boss->GetPositionX() + cos(angle) * 4.0f;
            float y = boss->GetPositionY() + sin(angle) * 4.0f;
            float z = boss->GetPositionZ();
            
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
        else
        {
            // Ranged maintain good positioning
            float angle = bot->GetAngle(boss);
            float x = boss->GetPositionX() + cos(angle) * 18.0f;
            float y = boss->GetPositionY() + sin(angle) * 18.0f;
            float z = boss->GetPositionZ();
            
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool AvoidWingBuffetAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Temporus is present and positioning is needed
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_TEMPORUS)
        {
            float distance = bot->GetDistance(unit);
            // Check if positioning is suboptimal
            return (distance > 5.0f && distance < 15.0f);
        }
    }
    
    return false;
}