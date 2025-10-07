#include "SethekkHallsActions.h"
#include "Group.h"
#include "Value.h"
#include "Playerbots.h"
#include "AttackersValue.h"
#include "strategy/dungeons/tbc/TbcDungeonHelpers.h"

std::map<ObjectGuid, uint32> g_ikiss_lastMoveTime;
std::map<ObjectGuid, bool> g_ikiss_inSafePosition;

bool AttackCharmingTotemAction::Execute(Event event)
{
    if (bot->IsCharmed())
        return false;

    Unit* totem = nullptr;

    // ICC Pattern (RaidIccTriggers.cpp:301-312): No IsInCombat check for spawned adds
    // RESEARCHED: Totems may not be flagged as "in combat" when first spawned
    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (const auto& npc : npcs)
    {
        if (Unit* unit = botAI->GetUnit(npc))
        {
            if (unit->GetEntry() == NPC_CHARMING_TOTEM && unit->IsAlive())
            {
                totem = unit;
                break;
            }
        }
    }

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    // Prevent ping-pong between multiple totems if attacking one already
    if (totem && currentTarget && currentTarget->GetEntry() == NPC_CHARMING_TOTEM)
    {
        return false;
    }

    if (!totem || AI_VALUE(Unit*, "current target") == totem)
    {
        return false;
    }

    return Attack(totem);
}

bool AttackCharmingTotemAction::isUseful()
{
    return !botAI->IsHeal(bot) && !bot->IsCharmed();
}

bool InterruptControllerAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    bool interrupted = false;
    TbcDungeon::ForEachNearbyNpc(botAI, bot, SEARCH_RANGE_MEDIUM, [&](Unit* unit)
    {
        if (interrupted || unit->GetEntry() != NPC_TIME_LOST_CONTROLLER || !unit->IsInCombat())
            return;

        if (!unit->HasUnitState(UNIT_STATE_CASTING) || !unit->FindCurrentSpellBySpellId(SPELL_SUMMON_TOTEM))
            return;

        if (Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "interrupt"))
        {
            for (uint32 spellId : spellIdsValue->Get())
            {
                if (botAI->CanCastSpell(spellId, unit, false) && botAI->CastSpell(spellId, unit))
                {
                    interrupted = true;
                    return;
                }
            }
        }
    });

    return interrupted;
}

bool InterruptControllerAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    bool interruptNeeded = false;
    TbcDungeon::ForEachNearbyNpc(botAI, bot, SEARCH_RANGE_MEDIUM, [&](Unit* unit)
    {
        if (interruptNeeded)
            return;

        if (unit->GetEntry() != NPC_TIME_LOST_CONTROLLER || !unit->IsInCombat())
            return;

        if (unit->HasUnitState(UNIT_STATE_CASTING) && unit->FindCurrentSpellBySpellId(SPELL_SUMMON_TOTEM))
            interruptNeeded = true;
    });

    return interruptNeeded;
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

    Unit* closestSpirit = nullptr;
    float closestDistance = SEARCH_RANGE_SMALL;
    TbcDungeon::ForEachNearbyNpc(botAI, bot, SEARCH_RANGE_SMALL, [&](Unit* unit)
    {
        if (unit->GetEntry() != NPC_SETHEKK_SPIRIT)
            return;

        float distance = bot->GetDistance(unit);
        if (distance < closestDistance)
        {
            closestDistance = distance;
            closestSpirit = unit;
        }
    });

    if (!closestSpirit)
        return false;

    // Prefer collision-aware flee helpers to avoid cutting through walls/objects
    // Try a short, safe step away using MoveAway (checks LOS/collision and picks side angles)
    const float fleeStep = 12.0f; // shorter controlled step within the corridor/room
    if (MoveAway(closestSpirit, fleeStep))
        return true;

    // Fallback: use FleeManager-based positioning from the spirit’s location
    return FleePosition(closestSpirit->GetPosition(), fleeStep, 600U);
}

bool FleeSpiritAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    bool spiritNearby = false;
    TbcDungeon::ForEachNearbyNpc(botAI, bot, SEARCH_RANGE_SMALL, [&](Unit* unit)
    {
        if (unit->GetEntry() == NPC_SETHEKK_SPIRIT)
            spiritNearby = true;
    });

    return spiritNearby;
}

bool AttackBroodOfAnzuAction::Execute(Event event)
{
    Unit* brood = nullptr;

    // ICC Pattern (RaidIccTriggers.cpp:301-312): No IsInCombat check for spawned adds
    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (const auto& npc : npcs)
    {
        if (Unit* unit = botAI->GetUnit(npc))
        {
            if (unit->GetEntry() == NPC_BROOD_OF_ANZU && unit->IsAlive())
            {
                brood = unit;
                break;
            }
        }
    }

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    // Prevent ping-pong between multiple brood if attacking one already
    if (brood && currentTarget && currentTarget->GetEntry() == NPC_BROOD_OF_ANZU)
    {
        return false;
    }

    if (!brood || AI_VALUE(Unit*, "current target") == brood)
    {
        return false;
    }

    return Attack(brood);
}

bool AttackBroodOfAnzuAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    if (botAI->IsHeal(bot))
        return false;

    bool targetAvailable = false;
    TbcDungeon::ForEachNearbyNpc(botAI, bot, SEARCH_RANGE_LARGE, [&](Unit* unit)
    {
        if (targetAvailable)
            return;

        if (unit->GetEntry() == NPC_BROOD_OF_ANZU && AttackersValue::IsValidTarget(unit, bot))
            targetAvailable = true;
    });

    return targetAvailable;
}

bool ContinueFightWithCharmedAllyAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    if (bot->IsCharmed())
        return false;

    Unit* controller = nullptr;
    float closestDistance = SEARCH_RANGE_LARGE;
    TbcDungeon::ForEachNearbyNpc(botAI, bot, SEARCH_RANGE_LARGE, [&](Unit* unit)
    {
        if (unit->GetEntry() != NPC_TIME_LOST_CONTROLLER || !unit->IsInCombat() || unit->IsCharmed())
            return;

        if (!AttackersValue::IsValidTarget(unit, bot))
            return;

        float distance = bot->GetDistance(unit);
        if (distance < closestDistance)
        {
            closestDistance = distance;
            controller = unit;
        }
    });

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

    bool hasCharmedAlly = false;
    bool hasTotem = false;
    TbcDungeon::ForEachNearbyNpc(botAI, bot, SEARCH_RANGE_LARGE, [&](Unit* unit)
    {
        if (!unit->IsAlive())
            return;

        if (unit->IsPlayer() && bot->IsInSameGroupWith(unit->ToPlayer()) && unit->IsCharmed())
            hasCharmedAlly = true;

        if (unit->GetEntry() == NPC_CHARMING_TOTEM)
            hasTotem = true;
    });

    return hasCharmedAlly && !hasTotem;
}

bool AttackSythElementalsAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    
    Unit* elemental = nullptr;
    float closestDistance = SEARCH_RANGE_LARGE;
    TbcDungeon::ForEachNearbyNpc(botAI, bot, SEARCH_RANGE_LARGE, [&](Unit* unit)
    {
        if (!(unit->GetEntry() == NPC_SYTH_FIRE_ELEMENTAL ||
              unit->GetEntry() == NPC_SYTH_FROST_ELEMENTAL ||
              unit->GetEntry() == NPC_SYTH_ARCANE_ELEMENTAL ||
              unit->GetEntry() == NPC_SYTH_SHADOW_ELEMENTAL))
            return;

        if (!AttackersValue::IsValidTarget(unit, bot))
            return;

        float distance = bot->GetDistance(unit);
        if (distance < closestDistance)
        {
            closestDistance = distance;
            elemental = unit;
        }
    });

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

    bool elementalPresent = false;
    TbcDungeon::ForEachNearbyNpc(botAI, bot, SEARCH_RANGE_LARGE, [&](Unit* unit)
    {
        if (elementalPresent)
            return;

        if ((unit->GetEntry() == NPC_SYTH_FIRE_ELEMENTAL ||
             unit->GetEntry() == NPC_SYTH_FROST_ELEMENTAL ||
             unit->GetEntry() == NPC_SYTH_ARCANE_ELEMENTAL ||
             unit->GetEntry() == NPC_SYTH_SHADOW_ELEMENTAL) &&
            AttackersValue::IsValidTarget(unit, bot))
        {
            elementalPresent = true;
        }
    });

    return elementalPresent;
}
