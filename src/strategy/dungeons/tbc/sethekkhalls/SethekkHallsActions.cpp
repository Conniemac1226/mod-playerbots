#include "SethekkHallsActions.h"
#include "Group.h"
#include "Value.h"
#include "Playerbots.h"
#include "AttackersValue.h"
#include "strategy/dungeons/tbc/TbcDungeonHelpers.h"
#include <cmath>

std::map<ObjectGuid, uint32> g_ikiss_lastMoveTime;
std::map<ObjectGuid, bool> g_ikiss_inSafePosition;

namespace
{
bool IsSythElemental(uint32 entry)
{
    return entry == NPC_SYTH_FIRE_ELEMENTAL ||
           entry == NPC_SYTH_FROST_ELEMENTAL ||
           entry == NPC_SYTH_ARCANE_ELEMENTAL ||
           entry == NPC_SYTH_SHADOW_ELEMENTAL;
}

Unit* FindDarkweaverSyth(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot)
        return nullptr;

    AiObjectContext* context = botAI->GetAiObjectContext();
    if (context)
    {
        if (Unit* syth = context->GetValue<Unit*>("find target", "darkweaver syth")->Get())
        {
            if (syth->GetEntry() == NPC_DARKWEAVER_SYTH && syth->IsAlive())
                return syth;
        }
    }

    if (context)
    {
        if (Value<ObjectGuid>* targetValue = context->GetValue<ObjectGuid>("current target"))
        {
            ObjectGuid targetGuid = targetValue->Get();
            if (targetGuid)
            {
                if (Unit* target = botAI->GetUnit(targetGuid))
                {
                    if (target->GetEntry() == NPC_DARKWEAVER_SYTH && target->IsAlive())
                        return target;
                }
            }
        }
    }

    Unit* syth = nullptr;
    TbcDungeon::ForEachNearbyNpc(botAI, bot, SEARCH_RANGE_LARGE, [&](Unit* unit)
    {
        if (!syth && unit->GetEntry() == NPC_DARKWEAVER_SYTH && unit->IsAlive())
            syth = unit;
    });

    return syth;
}

bool HasActiveSythElementals(PlayerbotAI* botAI, Player* bot)
{
    bool hasElementals = false;
    TbcDungeon::ForEachNearbyNpc(botAI, bot, SEARCH_RANGE_LARGE, [&](Unit* unit)
    {
        if (hasElementals || !unit->IsAlive())
            return;

        if (IsSythElemental(unit->GetEntry()))
            hasElementals = true;
    });

    return hasElementals;
}

Player* FindMainTank(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot)
        return nullptr;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && member->GetMapId() == bot->GetMapId() && botAI->IsMainTank(member))
                return member;
        }
    }

    return botAI->IsMainTank(bot) ? bot : nullptr;
}

Unit* FindIkiss(PlayerbotAI* botAI)
{
    if (!botAI)
        return nullptr;

    AiObjectContext* context = botAI->GetAiObjectContext();
    if (!context)
        return nullptr;

    if (Unit* boss = context->GetValue<Unit*>("find target", "talon king ikiss")->Get())
        return boss->IsAlive() ? boss : nullptr;

    if (Value<ObjectGuid>* targetValue = context->GetValue<ObjectGuid>("current target"))
    {
        ObjectGuid targetGuid = targetValue->Get();
        if (targetGuid)
        {
            if (Unit* target = botAI->GetUnit(targetGuid))
            {
                if (target->GetEntry() == NPC_TALON_KING_IKISS && target->IsAlive())
                    return target;
            }
        }
    }

    return nullptr;
}

bool IsGroupReadyForAdvancePull(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot || !bot->GetMap() || !bot->GetMap()->IsDungeon())
        return false;

    if (!bot->GetGroup() || !bot->IsAlive() || bot->IsInCombat() || !botAI->IsMainTank(bot))
        return false;

    AiObjectContext* context = botAI->GetAiObjectContext();
    if (!context)
        return false;

    if (context->GetValue<uint8>("attacker count")->Get() != 0)
        return false;

    if (Unit* currentTarget = context->GetValue<Unit*>("current target")->Get())
    {
        if (currentTarget->IsAlive() && currentTarget->IsInWorld() && currentTarget->GetMapId() == bot->GetMapId())
            return false;
    }

    if (bot->HealthBelowPct(AUTO_PULL_TANK_HP_PCT))
        return false;

    bool foundHealer = false;
    bool healerReady = false;

    Group* group = bot->GetGroup();
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot)
            continue;

        if (!member->IsAlive() || member->IsBeingTeleported() || member->IsInCombat())
            return false;

        if (bot->GetMapId() != member->GetMapId() || bot->GetDistance(member) > AUTO_PULL_GROUP_RANGE)
            return false;

        if (member->HealthBelowPct(AUTO_PULL_MEMBER_HP_PCT))
            return false;

        if (!foundHealer && botAI->IsHeal(member))
        {
            foundHealer = true;
            healerReady = member->getPowerType() != POWER_MANA ||
                member->GetPowerPct(POWER_MANA) >= AUTO_PULL_HEALER_MANA_PCT;
        }
    }

    return !foundHealer || healerReady;
}

Unit* SelectAdvancePullTarget(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot)
        return nullptr;

    Unit* bestForwardTarget = nullptr;
    float bestForwardDistance = AUTO_PULL_SEARCH_RANGE;
    Unit* bestFallbackTarget = nullptr;
    float bestFallbackDistance = AUTO_PULL_SEARCH_RANGE;

    TbcDungeon::ForEachNearbyNpc(botAI, bot, AUTO_PULL_SEARCH_RANGE, [&](Unit* unit)
    {
        if (!unit || !unit->IsAlive() || unit->IsInCombat())
            return;

        if (!AttackersValue::IsPossibleTarget(unit, bot))
            return;

        if (!bot->IsWithinLOSInMap(unit))
            return;

        float const distance = bot->GetDistance(unit);
        if (bot->HasInArc(static_cast<float>(M_PI) * 0.75f, unit))
        {
            if (distance < bestForwardDistance)
            {
                bestForwardDistance = distance;
                bestForwardTarget = unit;
            }
        }
        else if (distance < bestFallbackDistance)
        {
            bestFallbackDistance = distance;
            bestFallbackTarget = unit;
        }
    });

    return bestForwardTarget ? bestForwardTarget : bestFallbackTarget;
}
}

bool MarkCharmingTotemAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot || bot->IsCharmed())
        return false;

    Unit* totem = nullptr;

    // ICC Pattern (RaidIccActions.cpp:1254-1274): Find priority target using GuidVector
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

    if (!totem)
        return false;

    // ICC Pattern (RaidIccActions.cpp:1276-1297): Update skull marker on priority target
    UpdateSkullMarker(totem);

    return false;
}

void MarkCharmingTotemAction::UpdateSkullMarker(Unit* totem)
{
    if (!totem)
        return;

    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group)
        return;

    constexpr uint8_t skullIconId = 7;

    // Get current skull target
    ObjectGuid currentSkull = group->GetTargetIcon(skullIconId);
    Unit* currentSkullUnit = botAI->GetUnit(currentSkull);

    // Determine if skull marker needs updating
    bool needsUpdate = !currentSkullUnit || !currentSkullUnit->IsAlive() || currentSkullUnit != totem;

    // Update if needed
    if (needsUpdate)
        group->SetTargetIcon(skullIconId, bot->GetGUID(), totem->GetGUID());
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
        return MoveTo(bot->GetMapId(), IKISS_TANK_ANCHOR_POSITION.GetPositionX(),
                      IKISS_TANK_ANCHOR_POSITION.GetPositionY(), IKISS_TANK_ANCHOR_POSITION.GetPositionZ(),
                      false, false, false, true, MovementPriority::MOVEMENT_FORCED);
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

    Unit* closestSpirit = FindThreateningSpirit(bot);
    if (!closestSpirit)
        return false;

    if (bot->GetExactDist2d(closestSpirit) >= SETHEKK_SPIRIT_SAFE_RANGE)
        return false;

    if (MoveAway(closestSpirit, SETHEKK_SPIRIT_FLEE_STEP))
        return true;

    return FleePosition(closestSpirit->GetPosition(), SETHEKK_SPIRIT_FLEE_STEP, 400U);
}

bool FleeSpiritAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* spirit = FindThreateningSpirit(bot);
    return spirit && bot->GetExactDist2d(spirit) < SETHEKK_SPIRIT_SAFE_RANGE;
}

Unit* FleeSpiritAction::FindThreateningSpirit(Player* bot) const
{
    if (!bot)
        return nullptr;

    Unit* closestSpirit = nullptr;
    float closestDistance = SEARCH_RANGE_SMALL;
    TbcDungeon::ForEachNearbyNpc(botAI, bot, SEARCH_RANGE_SMALL, [&](Unit* unit)
    {
        if (unit->GetEntry() != NPC_SETHEKK_SPIRIT || !unit->IsAlive())
            return;

        if (unit->GetVictim() != bot && unit->GetTarget() != bot->GetGUID())
            return;

        float distance = bot->GetDistance(unit);
        if (distance < closestDistance)
        {
            closestDistance = distance;
            closestSpirit = unit;
        }
    });

    return closestSpirit;
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

bool StackForSythAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot || !bot->IsAlive() || botAI->IsTank(bot))
        return false;

    Unit* syth = FindDarkweaverSyth(botAI, bot);
    if (!syth || !syth->IsInCombat() || HasActiveSythElementals(botAI, bot))
        return false;

    float stackRange = botAI->IsRanged(bot) || botAI->IsHeal(bot) ? SYTH_STACK_RANGE : 4.0f;
    if (bot->GetExactDist2d(syth) <= stackRange + 1.0f)
        return false;

    Group* group = bot->GetGroup();
    uint32 groupIndex = 0;
    if (group)
    {
        uint32 currentIndex = 0;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || botAI->IsTank(member))
                continue;

            if (member == bot)
            {
                groupIndex = currentIndex;
                break;
            }

            ++currentIndex;
        }
    }

    float angle = syth->GetAngle(bot) + (groupIndex % 6) * (static_cast<float>(M_PI) / 6.0f);
    float x = syth->GetPositionX() + std::cos(angle) * stackRange;
    float y = syth->GetPositionY() + std::sin(angle) * stackRange;
    float z = syth->GetPositionZ();
    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
}

bool StackForSythAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !bot->IsAlive() || botAI->IsTank(bot))
        return false;

    Unit* syth = FindDarkweaverSyth(botAI, bot);
    if (!syth || !syth->IsInCombat() || HasActiveSythElementals(botAI, bot))
        return false;

    float stackRange = botAI->IsRanged(bot) || botAI->IsHeal(bot) ? SYTH_STACK_RANGE : 4.0f;
    return bot->GetExactDist2d(syth) > stackRange + 1.0f;
}

bool SethekkFearWardTankAction::Execute(Event /*event*/)
{
    Player* bot = botAI->GetBot();
    if (!bot || bot->getClass() != CLASS_PRIEST)
        return false;

    Player* tank = FindMainTank(botAI, bot);
    if (!tank || tank->HasAura(6346) || bot->GetDistance(tank) > SETHEKK_ANTI_FEAR_RANGE)
        return false;

    if (botAI->CanCastSpell("fear ward", tank))
        return botAI->CastSpell("fear ward", tank);

    return false;
}

bool SethekkTremorTotemAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot || bot->getClass() != CLASS_SHAMAN)
        return false;

    if (AI_VALUE2(bool, "has totem", "tremor"))
        return false;

    if (botAI->DoSpecificAction("tremor totem", event, true))
        return true;

    if (botAI->CanCastSpell(8143, bot, false))
        return botAI->CastSpell(8143, bot);

    return false;
}

bool IkissTankPillarPositionAction::Execute(Event /*event*/)
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI->IsMainTank(bot))
        return false;

    Unit* boss = FindIkiss(botAI);
    if (!boss || !boss->IsInCombat() || boss->HasAura(SPELL_ARCANE_BUBBLE))
        return false;

    if (bot->GetExactDist2d(IKISS_TANK_ANCHOR_POSITION) <= IKISS_TANK_ANCHOR_RANGE)
        return false;

    return MoveTo(bot->GetMapId(), IKISS_TANK_ANCHOR_POSITION.GetPositionX(),
                  IKISS_TANK_ANCHOR_POSITION.GetPositionY(), IKISS_TANK_ANCHOR_POSITION.GetPositionZ(),
                  false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
}

bool IkissTankPillarPositionAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !bot->IsAlive() || !botAI->IsMainTank(bot))
        return false;

    Unit* boss = FindIkiss(botAI);
    return boss && boss->IsInCombat() && !boss->HasAura(SPELL_ARCANE_BUBBLE) &&
           bot->GetExactDist2d(IKISS_TANK_ANCHOR_POSITION) > IKISS_TANK_ANCHOR_RANGE;
}

bool SethekkTankAdvancePullAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!IsGroupReadyForAdvancePull(botAI, bot))
        return false;

    Unit* target = SelectAdvancePullTarget(botAI, bot);
    if (!target)
        return false;

    context->GetValue<Unit*>("current target")->Set(target);
    bot->SetSelection(target->GetGUID());

    bool usedRangedOpener = false;
    if (botAI->IsTank(bot))
    {
        if (bot->getClass() == CLASS_PALADIN)
        {
            usedRangedOpener = botAI->DoSpecificAction("avenger's shield", event, true) ||
                               botAI->DoSpecificAction("hand of reckoning", event, true);
        }
        else if (bot->getClass() == CLASS_WARRIOR)
        {
            usedRangedOpener = botAI->DoSpecificAction("heroic throw", event, true);
        }
    }

    bool attackStarted = Attack(target);
    if (usedRangedOpener || attackStarted)
    {
        botAI->SetNextCheckDelay(sPlayerbotAIConfig.reactDelay);
        return true;
    }

    return false;
}

bool SethekkTankAdvancePullAction::isUseful()
{
    Player* bot = botAI->GetBot();
    return IsGroupReadyForAdvancePull(botAI, bot) && SelectAdvancePullTarget(botAI, bot);
}
