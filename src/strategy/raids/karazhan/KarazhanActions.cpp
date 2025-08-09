#include "KarazhanActions.h"
#include "KarazhanTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Player.h"
#include "Unit.h"
#include "Creature.h"
#include "SharedDefines.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"

bool AttumenAvoidChargeAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find Attumen Mounted
    Unit* boss = AI_VALUE2(Unit*, "find target", "attumen the huntsman");
    if (!boss)
        return false;

    // During mounted phase, stay within 8 yards (melee) or beyond 25 yards (ranged)
    // to avoid charge which targets players between 8-25 yards
    float distance = bot->GetDistance(boss);
    
    // If we're in the danger zone (8-25 yards), move appropriately
    if (distance > 8.0f && distance < 25.0f)
    {
        // Melee should move closer
        if (botAI->IsMelee(bot))
        {
            // Move to 5 yards (safe melee range)
            float angle = bot->GetAngle(boss);
            float x = boss->GetPositionX() + cos(angle) * 5.0f;
            float y = boss->GetPositionY() + sin(angle) * 5.0f;
            float z = boss->GetPositionZ();
            
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
        // Ranged should move further away
        else
        {
            // Move to 30 yards (safe ranged distance)
            float angle = bot->GetAngle(boss);
            float x = bot->GetPositionX() + cos(angle) * 10.0f;  // Move 10 yards further back
            float y = bot->GetPositionY() + sin(angle) * 10.0f;
            float z = bot->GetPositionZ();
            
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }
    
    return false;
}

bool AttumenAvoidChargeAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find the mounted boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "attumen the huntsman");
    if (!boss || boss->GetEntry() != NPC_ATTUMEN_MOUNTED)
        return false;

    // Check if we're in the danger zone for charge (8-25 yards)
    float distance = bot->GetDistance(boss);
    return (distance > 8.0f && distance < 25.0f);
}

bool AttumenPositionAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find Attumen (unmounted)
    Unit* attumen = nullptr;
    Unit* midnight = nullptr;
    
    // Search for both bosses
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_ATTUMEN_UNMOUNTED)
            attumen = unit;
        else if (unit->GetEntry() == NPC_MIDNIGHT)
            midnight = unit;
    }

    // During phase 2 (both bosses up), tanks need to separate them
    if (attumen && midnight)
    {
        // Tanks should position bosses apart to avoid cleave hitting both
        if (botAI->IsTank(bot))
        {
            Unit* myTarget = bot->GetVictim();
            if (myTarget)
            {
                // If tanking Attumen, move him away from Midnight
                if (myTarget == attumen)
                {
                    float angle = attumen->GetAngle(midnight) + M_PI;  // Opposite direction from Midnight
                    float x = attumen->GetPositionX() + cos(angle) * 10.0f;
                    float y = attumen->GetPositionY() + sin(angle) * 10.0f;
                    float z = attumen->GetPositionZ();
                    
                    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
                }
            }
        }
        // Non-tanks should stay behind Attumen to avoid Shadowcleave
        else if (attumen)
        {
            // Position behind Attumen (if we're in front arc, we're in danger)
            if (bot->GetDistance(attumen) < 10.0f && attumen->HasInArc(M_PI / 2, bot))
            {
                float angle = attumen->GetOrientation() + M_PI;
                float x = attumen->GetPositionX() + cos(angle) * 5.0f;
                float y = attumen->GetPositionY() + sin(angle) * 5.0f;
                float z = attumen->GetPositionZ();
                
                return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
            }
        }
    }
    
    return false;
}

bool AttumenPositionAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check for phase 2 (both bosses up)
    bool hasAttumen = false;
    bool hasMidnight = false;
    
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_ATTUMEN_UNMOUNTED)
            hasAttumen = true;
        else if (unit->GetEntry() == NPC_MIDNIGHT)
            hasMidnight = true;
    }
    
    // Useful during phase 2 when both are up
    return hasAttumen && hasMidnight;
}