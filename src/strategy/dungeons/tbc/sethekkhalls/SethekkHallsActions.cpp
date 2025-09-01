#include "SethekkHallsActions.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "Value.h"
#include "Playerbots.h"
#include "AttackersValue.h"

std::map<ObjectGuid, uint32> g_ikiss_lastMoveTime;
std::map<ObjectGuid, bool> g_ikiss_inSafePosition;

bool AttackCharmingTotemAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    if (bot->IsCharmed())
        return false;

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, SEARCH_RANGE_LARGE);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, SEARCH_RANGE_LARGE);

    Unit* totem = nullptr;
    float closestDistance = SEARCH_RANGE_LARGE;
    bool hasCharmedAlly = false;

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->IsPlayer() && bot->IsInSameGroupWith(unit->ToPlayer()) && unit->IsCharmed())
        {
            hasCharmedAlly = true;
        }

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

    if (bot->IsCharmed())
        return false;

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, SEARCH_RANGE_LARGE);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, SEARCH_RANGE_LARGE);

    bool hasTotem = false;
    bool hasCharmedAlly = false;

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_CHARMING_TOTEM && AttackersValue::IsValidTarget(unit, bot))
        {
            hasTotem = true;
        }

        if (unit->IsPlayer() && bot->IsInSameGroupWith(unit->ToPlayer()) && unit->IsCharmed())
        {
            hasCharmedAlly = true;
        }
    }
    
    return hasTotem;
}

bool InterruptControllerAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, SEARCH_RANGE_MEDIUM);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, SEARCH_RANGE_MEDIUM);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_TIME_LOST_CONTROLLER && unit->IsInCombat())
        {
            if (unit->HasUnitState(UNIT_STATE_CASTING) && unit->FindCurrentSpellBySpellId(SPELL_SUMMON_TOTEM))
            {
                Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "interrupt");
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

bool InterruptControllerAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, SEARCH_RANGE_MEDIUM);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, SEARCH_RANGE_MEDIUM);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_TIME_LOST_CONTROLLER && unit->IsInCombat())
        {
            if (unit->HasUnitState(UNIT_STATE_CASTING) && unit->FindCurrentSpellBySpellId(SPELL_SUMMON_TOTEM))
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

    uint32 currentTime = getMSTime();
    ObjectGuid botGuid = bot->GetGUID();
    
    
    Unit* boss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    if (!boss) {
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

    if (g_ikiss_inSafePosition[botGuid] && boss->HasAura(SPELL_ARCANE_BUBBLE)) {
        if ((currentTime - g_ikiss_lastMoveTime[botGuid]) > 10000) { // 10+ seconds = new phase
            g_ikiss_inSafePosition[botGuid] = false;
            g_ikiss_lastMoveTime[botGuid] = 0;
        } else {
            return true;
        }
    }
    
    if (g_ikiss_inSafePosition[botGuid] && !boss->HasAura(SPELL_ARCANE_BUBBLE)) {
        g_ikiss_inSafePosition[botGuid] = false;
        g_ikiss_lastMoveTime[botGuid] = 0;
        return false; // Allow normal movement logic to continue
    }


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
    
    bool result = MoveTo(bot->GetMapId(), closestSafePos->GetPositionX(), closestSafePos->GetPositionY(), closestSafePos->GetPositionZ(), 
                        false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    
    if (!result) {
        for (auto& altPos : IKISS_LOS_SAFE_POSITIONS) {
            if (&altPos == closestSafePos) continue; // Skip the one we just tried
            
            bool altResult = MoveTo(bot->GetMapId(), altPos.GetPositionX(), altPos.GetPositionY(), altPos.GetPositionZ(), 
                                   false, false, false, true, MovementPriority::MOVEMENT_FORCED);
            
            if (altResult) {
                g_ikiss_lastMoveTime[botGuid] = currentTime;
                g_ikiss_inSafePosition[botGuid] = true;
                return true;
            }
        }
        
        bot->GetMotionMaster()->Clear();
        bot->StopMoving();
        
        if (boss) {
            Position centerPos = boss->GetPosition();
            bool centerResult = MoveTo(bot->GetMapId(), centerPos.GetPositionX(), centerPos.GetPositionY(), centerPos.GetPositionZ(),
                                      false, false, false, true, MovementPriority::MOVEMENT_FORCED);
            
            if (centerResult) {
                bool retryResult = MoveTo(bot->GetMapId(), closestSafePos->GetPositionX(), closestSafePos->GetPositionY(), closestSafePos->GetPositionZ(),
                                         false, false, false, true, MovementPriority::MOVEMENT_FORCED);
                                         
                if (retryResult) {
                    g_ikiss_lastMoveTime[botGuid] = currentTime;
                    g_ikiss_inSafePosition[botGuid] = true;
                    return true;
                }
            }
        }
        
        g_ikiss_inSafePosition[botGuid] = false;
        return false;
    }
    
    g_ikiss_lastMoveTime[botGuid] = currentTime;
    g_ikiss_inSafePosition[botGuid] = true;
    return result;
}

bool IkissMoveAwayAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    if (!boss)
        return false;

    return boss->HasAura(SPELL_ARCANE_BUBBLE); // SPELL_ARCANE_BUBBLE from boss_talon_king_ikiss.cpp:39
}

bool IkissReturnPositionAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    ObjectGuid botGuid = bot->GetGUID();
    
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

    float x = boss->GetPositionX();
    float y = boss->GetPositionY();
    float z = boss->GetPositionZ();
    
    bool isTank = botAI->IsTank(bot);
    
    if (isTank) {
        float centerX = 46.5f;  // Center of Ikiss room
        float centerY = 287.0f; // Center of Ikiss room  
        float centerZ = z;
        
        return MoveTo(bot->GetMapId(), centerX, centerY, centerZ, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }
    else {
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
        
        float angle = frand(0, 2 * M_PI);
        float newX = x + cos(angle) * range;
        float newY = y + sin(angle) * range;
        
        return MoveTo(bot->GetMapId(), newX, newY, z, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
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

    return !boss->HasAura(SPELL_ARCANE_BUBBLE); // SPELL_ARCANE_BUBBLE
}

bool FleeSpiritAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, SEARCH_RANGE_SMALL);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, SEARCH_RANGE_SMALL);

    Unit* closestSpirit = nullptr;
    float closestDistance = SEARCH_RANGE_SMALL;

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

    float angle = bot->GetAngle(closestSpirit) + M_PI;
    float x = bot->GetPositionX() + cos(angle) * SEARCH_RANGE_SMALL;
    float y = bot->GetPositionY() + sin(angle) * SEARCH_RANGE_SMALL;
    float z = bot->GetPositionZ();
    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                MovementPriority::MOVEMENT_FORCED);
}

bool FleeSpiritAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, SEARCH_RANGE_SMALL);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, SEARCH_RANGE_SMALL);

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

bool AttackBroodOfAnzuAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, SEARCH_RANGE_LARGE);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, SEARCH_RANGE_LARGE);

    Unit* brood = nullptr;
    float closestDistance = SEARCH_RANGE_LARGE;

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_BROOD_OF_ANZU && AttackersValue::IsValidTarget(unit, bot))
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                brood = unit;
                closestDistance = distance;
            }
        }
    }

    if (brood)
    {
        return Attack(brood);
    }
    
    return false;
}

bool AttackBroodOfAnzuAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    if (botAI->IsHeal(bot))
        return false;

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, SEARCH_RANGE_LARGE);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, SEARCH_RANGE_LARGE);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_BROOD_OF_ANZU && AttackersValue::IsValidTarget(unit, bot))
        {
            return true;
        }
    }
    return false;
}

bool ContinueFightWithCharmedAllyAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    if (bot->IsCharmed())
        return false;

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, SEARCH_RANGE_LARGE);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, SEARCH_RANGE_LARGE);

    Unit* controller = nullptr;
    float closestDistance = SEARCH_RANGE_LARGE;

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_TIME_LOST_CONTROLLER && unit->IsInCombat() && 
            !unit->IsCharmed() && AttackersValue::IsValidTarget(unit, bot))
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                controller = unit;
                closestDistance = distance;
            }
        }
    }

    if (controller)
    {
        return Attack(controller);
    }

    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && !target->IsCharmed() && AttackersValue::IsValidTarget(target, bot))
    {
        return Attack(target);
    }

    return false;
}

bool ContinueFightWithCharmedAllyAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || bot->IsCharmed())
        return false;

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, SEARCH_RANGE_LARGE);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, SEARCH_RANGE_LARGE);

    bool hasCharmedAlly = false;
    bool hasTotem = false;

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->IsPlayer() && bot->IsInSameGroupWith(unit->ToPlayer()) && unit->IsCharmed())
        {
            hasCharmedAlly = true;
        }

        if (unit->GetEntry() == NPC_CHARMING_TOTEM)
        {
            hasTotem = true;
        }
    }

    return hasCharmedAlly && !hasTotem;
}

bool AttackSythElementalsAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, SEARCH_RANGE_LARGE);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, SEARCH_RANGE_LARGE);

    Unit* elemental = nullptr;
    float closestDistance = SEARCH_RANGE_LARGE;

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if ((unit->GetEntry() == NPC_SYTH_FIRE_ELEMENTAL ||
             unit->GetEntry() == NPC_SYTH_FROST_ELEMENTAL ||
             unit->GetEntry() == NPC_SYTH_ARCANE_ELEMENTAL ||
             unit->GetEntry() == NPC_SYTH_SHADOW_ELEMENTAL) &&
            AttackersValue::IsValidTarget(unit, bot))
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                elemental = unit;
                closestDistance = distance;
            }
        }
    }

    if (elemental && currentTarget != elemental)
    {
        return Attack(elemental);
    }
    
    return false;
}

bool AttackSythElementalsAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    if (botAI->IsHeal(bot))
        return false;

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, SEARCH_RANGE_LARGE);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, SEARCH_RANGE_LARGE);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if ((unit->GetEntry() == NPC_SYTH_FIRE_ELEMENTAL ||
             unit->GetEntry() == NPC_SYTH_FROST_ELEMENTAL ||
             unit->GetEntry() == NPC_SYTH_ARCANE_ELEMENTAL ||
             unit->GetEntry() == NPC_SYTH_SHADOW_ELEMENTAL) &&
            AttackersValue::IsValidTarget(unit, bot))
        {
            return true;
        }
    }
    
    return false;
}



