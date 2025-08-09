#include "SethekkHallsActions.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "Playerbots.h"
#include "AttackersValue.h"

// Per-bot state maps for preventing repeated movements during Ikiss bubble phase
std::map<ObjectGuid, uint32> g_ikiss_lastMoveTime;
std::map<ObjectGuid, bool> g_ikiss_inSafePosition;

bool AttackCharmingTotemAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 50.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 50.0f);

    Unit* totem = nullptr;
    float closestDistance = 50.0f;

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_CHARMING_TOTEM && AttackersValue::IsValidTarget(unit, bot))
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                totem = unit;
                closestDistance = distance;
            }
        }
    }

    if (totem)
    {
        return Attack(totem);
    }
    
    return false;
}

bool AttackCharmingTotemAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 50.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 50.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_CHARMING_TOTEM && AttackersValue::IsValidTarget(unit, bot))
        {
            return true;
        }
    }
    return false;
}

bool InterruptControllerAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 30.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 30.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_TIME_LOST_CONTROLLER && unit->IsInCombat())
        {
            if (unit->HasUnitState(UNIT_STATE_CASTING) && unit->FindCurrentSpellBySpellId(32764))
            {
                std::list<uint32> spellIds = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "interrupt")->Get();
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
    return false;
}

bool InterruptControllerAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 30.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 30.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_TIME_LOST_CONTROLLER && unit->IsInCombat())
        {
            if (unit->HasUnitState(UNIT_STATE_CASTING) && unit->FindCurrentSpellBySpellId(32764))
            {
                return true;
            }
        }
    }
    return false;
}


bool IkissMoveAwayAction::Execute(Event event)
{
    
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // PREVENT REPEATED MOVEMENT: Check if we're already in a safe position
    uint32 currentTime = getMSTime();
    ObjectGuid botGuid = bot->GetGUID();
    
    
    Unit* boss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    if (!boss) {
        // Try alternative method
        AiObjectContext* context = botAI->GetAiObjectContext();
        if (context) {
            Value<ObjectGuid>* targetValue = context->GetValue<ObjectGuid>("current target");
            if (targetValue) {
                ObjectGuid targetGuid = targetValue->Get();
                if (targetGuid) {
                    Unit* target = botAI->GetUnit(targetGuid);
                    if (target && target->GetEntry() == NPC_TALON_KING_IKISS) {
                        boss = target;
                    }
                }
            }
        }
        
        if (!boss)
            return false;
    }

    // FORCE RESET: If we think we're safe but boss has bubble, this is a new bubble phase - reset state
    if (g_ikiss_inSafePosition[botGuid] && boss->HasAura(9438)) {
        // Check if enough time has passed since last move (new bubble phase)
        if ((currentTime - g_ikiss_lastMoveTime[botGuid]) > 10000) { // 10+ seconds = new phase
            g_ikiss_inSafePosition[botGuid] = false;
            g_ikiss_lastMoveTime[botGuid] = 0;
        } else {
            return true;
        }
    }
    
    // If boss doesn't have bubble and we think we're safe, reset state
    if (g_ikiss_inSafePosition[botGuid] && !boss->HasAura(9438)) {
        g_ikiss_inSafePosition[botGuid] = false;
        g_ikiss_lastMoveTime[botGuid] = 0;
        return false; // Allow normal movement logic to continue
    }


    // Find closest LoS safe position and move to it directly
    const Position* closestSafePos = nullptr;
    float closestDist = 999999.0f;
    
    for (auto& safePos : IKISS_LOS_SAFE_POSITIONS)
    {
        float dist = bot->GetExactDist(safePos);
        if (dist < closestDist)
        {
            closestSafePos = &safePos;
            closestDist = dist;
        }
    }
    
    if (!closestSafePos) {
        return false;
    }
    
    // Move directly to tested safe position - MoveTo handles pathfinding automatically
    bool result = MoveTo(bot->GetMapId(), closestSafePos->GetPositionX(), closestSafePos->GetPositionY(), closestSafePos->GetPositionZ(), 
                        false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    
    // If MoveTo fails, try alternative safe positions
    if (!result) {
        for (auto& altPos : IKISS_LOS_SAFE_POSITIONS) {
            if (&altPos == closestSafePos) continue; // Skip the one we just tried
            
            bool altResult = MoveTo(bot->GetMapId(), altPos.GetPositionX(), altPos.GetPositionY(), altPos.GetPositionZ(), 
                                   false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
            
            if (altResult) {
                g_ikiss_lastMoveTime[botGuid] = currentTime;
                g_ikiss_inSafePosition[botGuid] = true;
                return true;
            }
        }
        
        // EMERGENCY RESET: Try clearing movement and repositioning bot
        bot->GetMotionMaster()->Clear();
        bot->StopMoving();
        
        // Try moving to boss center first, then to closest safe position
        if (boss) {
            Position centerPos = boss->GetPosition();
            bool centerResult = MoveTo(bot->GetMapId(), centerPos.GetPositionX(), centerPos.GetPositionY(), centerPos.GetPositionZ(),
                                      false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
            
            if (centerResult) {
                // After reaching center, try closest safe position again
                bool retryResult = MoveTo(bot->GetMapId(), closestSafePos->GetPositionX(), closestSafePos->GetPositionY(), closestSafePos->GetPositionZ(),
                                         false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
                                         
                if (retryResult) {
                    g_ikiss_lastMoveTime[botGuid] = currentTime;
                    g_ikiss_inSafePosition[botGuid] = true;
                    return true;
                }
            }
        }
        
        // Final fallback - bot will take damage but won't spam failed movements
        g_ikiss_inSafePosition[botGuid] = false;
        return false;
    }
    
    // Movement succeeded
    g_ikiss_lastMoveTime[botGuid] = currentTime;
    g_ikiss_inSafePosition[botGuid] = true;
    return result;
}

bool IkissMoveAwayAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Pattern from HallsOfLightningActions.cpp:158-159
    Unit* boss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    if (!boss)
        return false;

    // Only useful when boss has arcane bubble (preparing explosion)
    return boss->HasAura(9438); // SPELL_ARCANE_BUBBLE from boss_talon_king_ikiss.cpp:39
}

bool IkissReturnPositionAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    ObjectGuid botGuid = bot->GetGUID();
    
    // Reset the safe position state when returning to combat
    g_ikiss_inSafePosition[botGuid] = false;
    g_ikiss_lastMoveTime[botGuid] = 0;

    AiObjectContext* context = botAI->GetAiObjectContext();
    if (!context)
        return false;

    Value<ObjectGuid>* targetValue = context->GetValue<ObjectGuid>("current target");
    if (!targetValue)
        return false;

    ObjectGuid targetGuid = targetValue->Get();
    if (!targetGuid)
        return false;

    Unit* boss = botAI->GetUnit(targetGuid);
    if (!boss || boss->GetEntry() != NPC_TALON_KING_IKISS)
        return false;

    // Return to boss position for normal combat positioning
    float x = boss->GetPositionX();
    float y = boss->GetPositionY();
    float z = boss->GetPositionZ();
    
    // TANK POSITIONING: If bot is tank, move boss back to room center for next phase
    bool isTank = botAI->IsTank(bot);
    
    if (isTank) {
        // Move to room center to pull boss away from pillars
        float centerX = 46.5f;  // Center of Ikiss room
        float centerY = 287.0f; // Center of Ikiss room  
        float centerZ = z;
        
        return MoveTo(bot->GetMapId(), centerX, centerY, centerZ, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }
    else {
        // Non-tanks: Position at appropriate range for class role
        float range;
        if (bot->getClass() == CLASS_WARRIOR || bot->getClass() == CLASS_PALADIN || 
            bot->getClass() == CLASS_ROGUE || bot->getClass() == CLASS_DEATH_KNIGHT)
        {
            range = 5.0f;  // Melee range
        }
        else
        {
            range = 18.0f; // Ranged/caster range
        }
        
        // Spread out around the boss/center point
        float angle = frand(0, 2 * M_PI);
        float newX = x + cos(angle) * range;
        float newY = y + sin(angle) * range;
        
        return MoveTo(bot->GetMapId(), newX, newY, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }
}

bool IkissReturnPositionAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    AiObjectContext* context = botAI->GetAiObjectContext();
    if (!context)
        return false;

    Value<ObjectGuid>* targetValue = context->GetValue<ObjectGuid>("current target");
    if (!targetValue)
        return false;

    ObjectGuid targetGuid = targetValue->Get();
    if (!targetGuid)
        return false;

    Unit* boss = botAI->GetUnit(targetGuid);
    if (!boss || boss->GetEntry() != NPC_TALON_KING_IKISS)
        return false;

    // Only useful when not in arcane bubble phase
    return !boss->HasAura(9438); // SPELL_ARCANE_BUBBLE
}

bool FleeSpiritAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find all Sethekk Spirits within range
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 40.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 40.0f);

    Unit* closestSpirit = nullptr;
    float closestDistance = 40.0f;

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_SETHEKK_SPIRIT)
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                closestSpirit = unit;
                closestDistance = distance;
            }
        }
    }

    if (!closestSpirit)
        return false;

    // Only flee if spirit is too close (within 15 yards)
    if (closestDistance > 15.0f)
        return false;

    // Use the FleePosition method from MovementAction base class
    // Flee at least 20 yards away from the spirit with 500ms minimum interval
    return FleePosition(closestSpirit->GetPosition(), 20.0f, 500U);
}

bool FleeSpiritAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if any Sethekk Spirit is within dangerous range (20 yards)
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 20.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 20.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_SETHEKK_SPIRIT)
            return true;
    }
    
    return false;
}