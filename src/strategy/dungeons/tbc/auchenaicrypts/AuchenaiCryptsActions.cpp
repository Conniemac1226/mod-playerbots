#include "AuchenaiCryptsActions.h"
#include "AuchenaiCryptsTriggers.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "Playerbots.h"
#include "AttackersValue.h"

// Per-bot state map for preventing repeated movements during Focus Fire phase
std::map<ObjectGuid, bool> g_shirrak_inSafePosition;

bool ShirrakFocusFireAvoidAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    ObjectGuid botGuid = bot->GetGUID();

    // Focus Fire is summoned at player position
    // Find Focus Fire creature location
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 60.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 60.0f);

    Unit* focusFire = nullptr;
    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_FOCUS_FIRE)
        {
            focusFire = unit;
            break;
        }
    }

    if (!focusFire)
        return false;

    // Already in safe position for this phase
    if (g_shirrak_inSafePosition[botGuid])
        return true;

    float focusX = focusFire->GetPositionX();
    float focusY = focusFire->GetPositionY();
    float focusZ = focusFire->GetPositionZ();

    // Calculate safe position away from Focus Fire
    float botX = bot->GetPositionX();
    float botY = bot->GetPositionY();
    float botZ = bot->GetPositionZ();
    
    // Calculate movement direction
    float dx = botX - focusX;
    float dy = botY - focusY;
    float distance = sqrt(dx * dx + dy * dy);
    
    if (distance < 0.1f) {
        dx = frand(-1.0f, 1.0f);
        dy = frand(-1.0f, 1.0f);
        distance = sqrt(dx * dx + dy * dy);
    }
    
    // Move 15 yards away
    float moveDistance = 15.0f;
    float safeX = focusX + (dx / distance) * moveDistance;
    float safeY = focusY + (dy / distance) * moveDistance;
    float safeZ = botZ;
    
    // Move to safe position
    bool result = MoveTo(bot->GetMapId(), safeX, safeY, safeZ, 
                        false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    
    if (!result) {
        // Try alternative position
        float altX = focusX - (dx / distance) * moveDistance;
        float altY = focusY - (dy / distance) * moveDistance;
        
        bool altResult = MoveTo(bot->GetMapId(), altX, altY, safeZ, 
                               false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        
        if (altResult) {
            g_shirrak_inSafePosition[botGuid] = true;
            return true;
        }
        g_shirrak_inSafePosition[botGuid] = false;
        return false;
    }
    
    g_shirrak_inSafePosition[botGuid] = true;
    return result;
}

bool ShirrakFocusFireAvoidAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Focus Fire exists
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 60.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 60.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_FOCUS_FIRE)
            return true;
    }
    
    return false;
}

bool ShirrakReturnPositionAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    ObjectGuid botGuid = bot->GetGUID();
    
    // Reset safe position state
    g_shirrak_inSafePosition[botGuid] = false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "shirrak the dead watcher");
    if (!boss)
        return false;

    // Return to optimal range based on class and Inhibit Magic stacks
    float optimalRange;
    if (bot->getClass() == CLASS_MAGE || bot->getClass() == CLASS_WARLOCK || 
        bot->getClass() == CLASS_PRIEST)
    {
        // Caster optimal range (2 stacks)
        optimalRange = 27.5f;
    }
    else if (bot->getClass() == CLASS_HUNTER)
    {
        // Hunter optimal range (1 stack)
        optimalRange = 37.5f;
    }
    else
    {
        // Melee optimal range (3 stacks)
        optimalRange = 17.5f;
    }
    
    // Position around boss at optimal range
    float angle = frand(0, 2 * M_PI);
    float newX = boss->GetPositionX() + cos(angle) * optimalRange;
    float newY = boss->GetPositionY() + sin(angle) * optimalRange;
    float newZ = boss->GetPositionZ();
    
    
    return MoveTo(bot->GetMapId(), newX, newY, newZ, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
}

bool ShirrakReturnPositionAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "shirrak the dead watcher");
    if (!boss)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    
    // Only useful if we were previously in safe position
    if (!g_shirrak_inSafePosition[botGuid])
        return false;

    // Only useful when no Focus Fire exists
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 60.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 60.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_FOCUS_FIRE)
            return false;
    }
    
    // Safe to return to combat position
    return true;
}