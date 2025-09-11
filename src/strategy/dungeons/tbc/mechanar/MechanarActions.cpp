#include "MechanarActions.h"
#include "MechanarTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "VMapFactory.h"

using namespace VMAP;

// Per-bot state management
std::map<ObjectGuid, uint32> g_capacitus_lastPolarityTime;
std::map<ObjectGuid, bool> g_capacitus_hasPositive;
std::map<ObjectGuid, bool> g_capacitus_hasNegative;
// Sepethrea kiting state
static std::map<ObjectGuid, uint32> g_sepethrea_lastKiteMove;
static std::map<ObjectGuid, float> g_sepethrea_kitePhase;
static std::map<ObjectGuid, int> g_sepethrea_kiteDir;          // +1 clockwise, -1 counter-clockwise
static std::map<ObjectGuid, uint32> g_sepethrea_blockedTries;  // consecutive failures

// Room boundaries for safe movement - Sepethrea room
const Position MECHANAR_SEPETHREA_CENTER = {290.52f, 11.492f, 25.39f, 0.0f};
const float MECHANAR_SEPETHREA_MIN_X = 272.0f;
const float MECHANAR_SEPETHREA_MAX_X = 308.0f;
const float MECHANAR_SEPETHREA_MIN_Y = -8.0f;
const float MECHANAR_SEPETHREA_MAX_Y = 31.0f;
const float MECHANAR_SEPETHREA_MIN_Z = 23.0f;
const float MECHANAR_SEPETHREA_MAX_Z = 28.0f;

// Room boundaries for safe movement - Pathaleon room 
const Position MECHANAR_PATHALEON_CENTER = {113.0f, -14.5f, 26.3f, 0.0f};
const float MECHANAR_PATHALEON_MIN_X = 95.0f;
const float MECHANAR_PATHALEON_MAX_X = 131.0f;
const float MECHANAR_PATHALEON_MIN_Y = -32.0f;
const float MECHANAR_PATHALEON_MAX_Y = 3.0f;
const float MECHANAR_PATHALEON_MIN_Z = 25.0f;
const float MECHANAR_PATHALEON_MAX_Z = 28.0f;

// Boundary validation to prevent out-of-bounds movement with buffer
bool IsPositionSafe(const Position& pos, PlayerbotAI* botAI = nullptr)
{
    const float buffer = 2.0f;
    
    // Detect which boss room we're in based on position or boss target
    bool isPathaleonRoom = false;
    if (botAI)
    {
        Unit* pathaleon = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "pathaleon the calculator")->Get();
        if (pathaleon && pathaleon->IsAlive())
            isPathaleonRoom = true;
    }
    
    // Use position heuristic if no AI context
    if (!isPathaleonRoom && !botAI)
    {
        isPathaleonRoom = (pos.m_positionX < 200.0f); // Pathaleon room is west of Sepethrea
    }
    
    if (isPathaleonRoom)
    {
        return pos.m_positionX >= MECHANAR_PATHALEON_MIN_X + buffer && pos.m_positionX <= MECHANAR_PATHALEON_MAX_X - buffer &&
               pos.m_positionY >= MECHANAR_PATHALEON_MIN_Y + buffer && pos.m_positionY <= MECHANAR_PATHALEON_MAX_Y - buffer &&
               pos.m_positionZ >= MECHANAR_PATHALEON_MIN_Z && pos.m_positionZ <= MECHANAR_PATHALEON_MAX_Z;
    }
    else
    {
        return pos.m_positionX >= MECHANAR_SEPETHREA_MIN_X + buffer && pos.m_positionX <= MECHANAR_SEPETHREA_MAX_X - buffer &&
               pos.m_positionY >= MECHANAR_SEPETHREA_MIN_Y + buffer && pos.m_positionY <= MECHANAR_SEPETHREA_MAX_Y - buffer &&
               pos.m_positionZ >= MECHANAR_SEPETHREA_MIN_Z && pos.m_positionZ <= MECHANAR_SEPETHREA_MAX_Z;
    }
}

// Validate movement path doesn't go through walls
bool IsPathClear(const Position& from, const Position& to, PlayerbotAI* botAI = nullptr)
{
    const int steps = 10; // Check 10 points along the path
    for (int i = 1; i <= steps; ++i)
    {
        float t = float(i) / float(steps);
        Position checkPos;
        checkPos.m_positionX = from.m_positionX + t * (to.m_positionX - from.m_positionX);
        checkPos.m_positionY = from.m_positionY + t * (to.m_positionY - from.m_positionY);
        checkPos.m_positionZ = from.m_positionZ + t * (to.m_positionZ - from.m_positionZ);
        
        if (!IsPositionSafe(checkPos, botAI))
            return false;
    }
    return true;
}

// Path hazard sampler: ensure path samples are not too close to any Raging Flames (avoids "trails")
bool IsPathSafeFromFlames(const Position& from, const Position& to, PlayerbotAI* botAI, float minDistance)
{
    if (!botAI)
        return true;

    const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
    std::vector<Unit*> flames;
    flames.reserve(8);
    for (auto const& guid : npcs)
    {
        Unit* u = botAI->GetUnit(guid);
        if (u && u->IsAlive() && u->GetEntry() == NPC_RAGING_FLAMES)
            flames.push_back(u);
    }
    if (flames.empty())
        return true;

    const int steps = 12;
    for (int i = 1; i <= steps; ++i)
    {
        float t = float(i) / float(steps);
        Position p;
        p.m_positionX = from.m_positionX + t * (to.m_positionX - from.m_positionX);
        p.m_positionY = from.m_positionY + t * (to.m_positionY - from.m_positionY);
        p.m_positionZ = from.m_positionZ + t * (to.m_positionZ - from.m_positionZ);
        for (Unit* f : flames)
        {
            if (p.GetExactDist2d(f) < minDistance)
                return false;
        }
    }
    return true;
}

// Force position to stay within safe room bounds
Position ConstrainToRoom(const Position& pos, PlayerbotAI* botAI = nullptr)
{
    const float buffer = 3.0f;
    Position safePos = pos;
    
    // Detect which boss room we're in
    bool isPathaleonRoom = false;
    if (botAI)
    {
        Unit* pathaleon = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "pathaleon the calculator")->Get();
        if (pathaleon && pathaleon->IsAlive())
            isPathaleonRoom = true;
    }
    
    // Use position heuristic if no AI context
    if (!isPathaleonRoom && !botAI)
    {
        isPathaleonRoom = (pos.m_positionX < 200.0f); // Pathaleon room is west of Sepethrea
    }
    
    if (isPathaleonRoom)
    {
        safePos.m_positionX = std::max(MECHANAR_PATHALEON_MIN_X + buffer, 
                                      std::min(MECHANAR_PATHALEON_MAX_X - buffer, pos.m_positionX));
        safePos.m_positionY = std::max(MECHANAR_PATHALEON_MIN_Y + buffer, 
                                      std::min(MECHANAR_PATHALEON_MAX_Y - buffer, pos.m_positionY));
        safePos.m_positionZ = std::max(MECHANAR_PATHALEON_MIN_Z, 
                                      std::min(MECHANAR_PATHALEON_MAX_Z, pos.m_positionZ));
    }
    else
    {
        safePos.m_positionX = std::max(MECHANAR_SEPETHREA_MIN_X + buffer, 
                                      std::min(MECHANAR_SEPETHREA_MAX_X - buffer, pos.m_positionX));
        safePos.m_positionY = std::max(MECHANAR_SEPETHREA_MIN_Y + buffer, 
                                      std::min(MECHANAR_SEPETHREA_MAX_Y - buffer, pos.m_positionY));
        safePos.m_positionZ = std::max(MECHANAR_SEPETHREA_MIN_Z, 
                                      std::min(MECHANAR_SEPETHREA_MAX_Z, pos.m_positionZ));
    }
    
    return safePos;
}

// Mechano Lord Capacitus

bool CapacitusReflectiveShieldAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "mechano lord capacitus");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Reflective shields - alternates between magic and damage shields
    if (boss->HasAura(SPELL_REFLECTIVE_MAGIC_SHIELD))
    {
        // Stop casting spells during magic shield
        if (bot->IsNonMeleeSpellCast(false))
        {
            botAI->InterruptSpell();
            return true;
        }
        // Melee can continue attacking
    }
    else if (boss->HasAura(SPELL_REFLECTIVE_DAMAGE_SHIELD))
    {
        // Melee should stop attacking during damage shield
        if (botAI->IsMelee(bot))
        {
            bot->AttackStop();
            return true;
        }
        // Casters can continue casting
    }

    return false;
}

bool CapacitusReflectiveShieldAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || bot->GetMap()->IsHeroic())
        return false; // Only on Normal

    Unit* boss = AI_VALUE2(Unit*, "find target", "mechano lord capacitus");
    return boss && (boss->HasAura(SPELL_REFLECTIVE_MAGIC_SHIELD) || 
                   boss->HasAura(SPELL_REFLECTIVE_DAMAGE_SHIELD));
}

bool CapacitusPolarityShiftAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    uint32 currentTime = getMSTime();

    bool hasPositive = bot->HasAura(SPELL_POSITIVE_POLARITY);
    bool hasNegative = bot->HasAura(SPELL_NEGATIVE_POLARITY);

    if (!hasPositive && !hasNegative)
    {
        g_capacitus_hasPositive[botGuid] = false;
        g_capacitus_hasNegative[botGuid] = false;
        return false;
    }

    g_capacitus_hasPositive[botGuid] = hasPositive;
    g_capacitus_hasNegative[botGuid] = hasNegative;
    g_capacitus_lastPolarityTime[botGuid] = currentTime;

    const GuidVector members = AI_VALUE(GuidVector, "group members");
    Unit* closestSamePolarityAlly = nullptr;
    float closestSamePolarityDistance = 100.0f;

    // Find the closest ally with the same polarity
    for (auto& member : members)
    {
        Unit* ally = botAI->GetUnit(member);
        if (!ally || ally == bot || !ally->IsAlive())
            continue;

        bool allyPositive = ally->HasAura(SPELL_POSITIVE_POLARITY);
        bool allyNegative = ally->HasAura(SPELL_NEGATIVE_POLARITY);

        if ((hasPositive && allyPositive) || (hasNegative && allyNegative))
        {
            float distance = bot->GetDistance(ally);
            if (distance < closestSamePolarityDistance)
            {
                closestSamePolarityAlly = ally;
                closestSamePolarityDistance = distance;
            }
        }
    }

    // Move to the closest ally with the same polarity if too far
    if (closestSamePolarityAlly && closestSamePolarityDistance > 8.0f)
    {
        return MoveTo(closestSamePolarityAlly, 4.0f);
    }

    // If close enough to an ally with the same polarity, stop moving
    if (closestSamePolarityAlly && closestSamePolarityDistance < 7.0f)
    {
        if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
        {
            bot->GetMotionMaster()->Clear();
            return true;
        }
    }

    // Move away from allies with opposite polarity
    for (auto& member : members)
    {
        Unit* ally = botAI->GetUnit(member);
        if (!ally || ally == bot || !ally->IsAlive())
            continue;

        bool allyPositive = ally->HasAura(SPELL_POSITIVE_POLARITY);
        bool allyNegative = ally->HasAura(SPELL_NEGATIVE_POLARITY);
        float distance = bot->GetDistance(ally);

        if ((hasPositive && allyNegative) || (hasNegative && allyPositive))
        {
            if (distance < 8.0f)
            {
                float angle = bot->GetAngle(ally) + M_PI;
                float x = bot->GetPositionX() + cos(angle) * 12.0f;
                float y = bot->GetPositionY() + sin(angle) * 12.0f;
                float z = bot->GetPositionZ();
                Position targetPos(x, y, z, 0.0f);
                targetPos = ConstrainToRoom(targetPos, botAI);

                if (IsPositionSafe(targetPos) && IsPathClear(bot->GetPosition(), targetPos))
                {
                    return MoveTo(bot->GetMapId(), targetPos.m_positionX, targetPos.m_positionY,
                                 targetPos.m_positionZ, false, false, false, true,
                                 MovementPriority::MOVEMENT_NORMAL);
                }
            }
        }
    }

    return false;
}

bool CapacitusPolarityShiftAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !bot->GetMap()->IsHeroic())
        return false; // Only on Heroic

    return bot->HasAura(SPELL_POSITIVE_POLARITY) || bot->HasAura(SPELL_NEGATIVE_POLARITY);
}

bool CapacitusNetherChargeAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Nether Charges - priority targets for ranged DPS
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* nearestCharge = nullptr;
    float closestDistance = 100.0f;

    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_NETHER_CHARGE)
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                nearestCharge = unit;
                closestDistance = distance;
            }
        }
    }

    if (nearestCharge)
    {
        // Priority target for ranged DPS
        if (!botAI->IsMelee(bot))
        {
            return Attack(nearestCharge);
        }
    }

    return false;
}

bool CapacitusNetherChargeAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* chargeActiveValue = botAI->GetAiObjectContext()->GetValue<bool>("nether charge active");
    if (!chargeActiveValue)
        return false;
    
    return chargeActiveValue->Get();
}

bool CapacitusPositionAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "mechano lord capacitus");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Spread positioning for Nether Charges
    const GuidVector members = AI_VALUE(GuidVector, "group members");
    
    for (auto& member : members)
    {
        Unit* ally = botAI->GetUnit(member);
        if (!ally || ally == bot || !ally->IsAlive())
            continue;

        float distance = bot->GetDistance(ally);
        if (distance < 8.0f && !botAI->IsTank(bot))
        {
            // Spread from allies for charges
            float angle = bot->GetAngle(ally) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 10.0f;
            float y = bot->GetPositionY() + sin(angle) * 10.0f;
            float z = bot->GetPositionZ();
            Position targetPos(x, y, z, 0.0f);
            targetPos = ConstrainToRoom(targetPos, botAI);
            
            if (IsPositionSafe(targetPos, botAI) && IsPathClear(bot->GetPosition(), targetPos, botAI))
            {
                return MoveTo(bot->GetMapId(), targetPos.m_positionX, targetPos.m_positionY,
                             targetPos.m_positionZ, false, false, false, true,
                             MovementPriority::MOVEMENT_NORMAL);
            }
            
            // Fallback: find safe direction
            for (int i = 0; i < 8; ++i)
            {
                float testAngle = (i * M_PI / 4);
                float testX = bot->GetPositionX() + cos(testAngle) * 8.0f;
                float testY = bot->GetPositionY() + sin(testAngle) * 8.0f;
                Position testPos(testX, testY, z, 0.0f);
                testPos = ConstrainToRoom(testPos, botAI);
                
                if (IsPositionSafe(testPos, botAI) && IsPathClear(bot->GetPosition(), testPos, botAI))
                {
                    return MoveTo(bot->GetMapId(), testPos.m_positionX, testPos.m_positionY,
                                 testPos.m_positionZ, false, false, false, true,
                                 MovementPriority::MOVEMENT_NORMAL);
                }
            }
        }
    }

    return false;
}

bool CapacitusPositionAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* engagedValue = botAI->GetAiObjectContext()->GetValue<bool>("capacitus engaged");
    if (!engagedValue)
        return false;
    
    return engagedValue->Get();
}

// ========== NETHERMANCER SEPETHREA ACTIONS ==========

bool SepethreaRagingFlamesAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find Raging Flames targeting this bot
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* targetingFlame = nullptr;
    
    for (auto& npc : npcs)
    {
        Unit* flame = botAI->GetUnit(npc);
        if (!flame || !flame->IsAlive() || flame->GetEntry() != NPC_RAGING_FLAMES)
            continue;
            
        if (flame->GetVictim() == bot)
        {
            targetingFlame = flame;
            break;
        }
    }
    
    if (!targetingFlame)
        return false;

    // MOVED: Spell interruption logic moved below for smarter handling

    // Emergency: immediate escape if standing in damaging aura
    if (bot->HasAura(SPELL_RAGING_FLAMES_AREA_AURA) || bot->HasAura(SPELL_INFERNO) || bot->HasAura(SPELL_INFERNO_DAMAGE))
    {
        return FleePosition(targetingFlame->GetPosition(), 22.0f, 900U);
    }

    // SMART KITING: Only interrupt long/unsafe casts
    if (bot->IsNonMeleeSpellCast(false))
    {
        if (Spell* currentSpell = bot->GetCurrentSpell(CURRENT_GENERIC_SPELL))
        {
            uint32 castTime = currentSpell->GetCastTime();
            float distanceToFlame = bot->GetDistance(targetingFlame);
            if (castTime > 1500 || distanceToFlame < 12.0f)
                botAI->InterruptSpell();
        }
        else
        {
            botAI->InterruptSpell();
        }
    }

    // CIRCULAR KITING: pick a tangential waypoint around room center to avoid backtracking through fire
    const Position& center = MECHANAR_SEPETHREA_CENTER;
    ObjectGuid guid = bot->GetGUID();
    uint32 now = getMSTime();
    uint32& lastMove = g_sepethrea_lastKiteMove[guid];
    if (lastMove && now - lastMove < 400) // throttle pathing
        return false;

    // Base angle from flame -> bot, then rotate +/- 60 degrees with per-bot phase
    float baseAngle = atan2f(bot->GetPositionY() - center.GetPositionY(), bot->GetPositionX() - center.GetPositionX());
    float phase = g_sepethrea_kitePhase.count(guid) ? g_sepethrea_kitePhase[guid] : float((guid.GetCounter() % 6) - 3) * 0.12f;
    g_sepethrea_kitePhase[guid] = phase;
    int& dir = g_sepethrea_kiteDir[guid];
    if (dir == 0) dir = (guid.GetCounter() % 2 == 0) ? 1 : -1;

    // Evaluate candidate waypoints (tangential arc)
    float bestScore = -1.0f;
    Position bestPos;
    const bool isHealer = botAI->IsHeal(bot);
    const float radius = isHealer ? 24.0f : 20.0f;
    // Try biased to current direction first to prevent zig-zag at walls
    for (int step = 0; step <= 4; ++step)
    {
        int i = dir * (step == 0 ? 1 : step); // 1,2,3,4 in current dir
        float ang = baseAngle + phase + i * 0.35f; // spread candidates along arc
        Position p;
        p.m_positionX = center.GetPositionX() + cosf(ang) * radius;
        p.m_positionY = center.GetPositionY() + sinf(ang) * radius;
        p.m_positionZ = bot->GetPositionZ();
        p = ConstrainToRoom(p, botAI);

        if (!IsPositionSafe(p, botAI) || !IsPathClear(bot->GetPosition(), p, botAI) ||
            !IsPathSafeFromFlames(bot->GetPosition(), p, botAI, 10.0f))
            continue;

        // Score by distance from all flames and away from boss frontal (for breath)
        float minFlameDist = 9999.0f;
        const GuidVector npcs2 = AI_VALUE(GuidVector, "nearest hostile npcs");
        for (auto& npc2 : npcs2)
        {
            Unit* u = botAI->GetUnit(npc2);
            if (u && u->IsAlive() && u->GetEntry() == NPC_RAGING_FLAMES)
            {
                float d = p.GetExactDist2d(u);
                if (d < minFlameDist) minFlameDist = d;
            }
        }
        if (minFlameDist < (isHealer ? 14.0f : 10.0f)) // too close to any flame
            continue;

        // Healers prefer staying within 35y of the nearest tank to keep healing range
        float rangePenalty = 0.0f;
        if (isHealer)
        {
            Unit* bestTank = nullptr; float bestTankD = 1e9f;
            const GuidVector members = AI_VALUE(GuidVector, "group members");
            for (auto& m : members)
            {
                Player* pl = botAI->GetPlayer(m);
                if (pl && pl->IsAlive() && botAI->IsTank(pl))
                {
                    float d = p.GetExactDist2d(pl);
                    if (d < bestTankD) { bestTankD = d; bestTank = pl; }
                }
            }
            if (bestTank)
            {
                if (bestTankD > 35.0f) rangePenalty = (bestTankD - 35.0f) * 0.5f; // penalize being too far to heal
            }
        }

        float score = minFlameDist - fabsf(i) * 1.5f - rangePenalty; // prefer further from flames, keep healer in range
        if (score > bestScore)
        {
            bestScore = score;
            bestPos = p;
        }
    }

    if (bestScore > 0.0f)
    {
        lastMove = now;
        return MoveTo(bot->GetMapId(), bestPos.m_positionX, bestPos.m_positionY, bestPos.m_positionZ,
                      false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }

    // Fallback: if blocked repeatedly, flip direction
    uint32& blocked = g_sepethrea_blockedTries[guid];
    blocked++;
    if (blocked > 3) { dir = -dir; blocked = 0; }

    // Fallback: flee directly if no good tangent found
    lastMove = now;
    return FleePosition(targetingFlame->GetPosition(), 20.0f, 1000U);
}

bool SepethreaRagingFlamesAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Use direct GUID search for flame detection
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    for (auto& npc : npcs)
    {
        Unit* flame = botAI->GetUnit(npc);
        if (!flame || !flame->IsAlive() || flame->GetEntry() != NPC_RAGING_FLAMES)
            continue;
            
        if (flame->GetVictim() == bot)
            return true;
    }
    
    return false;
}

bool SepethreaDragonsBreathAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "nethermancer sepethrea");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Dragon's Breath - frontal cone attack
    if (boss->HasUnitState(UNIT_STATE_CASTING) && 
        boss->FindCurrentSpellBySpellId(MECH_SPELL_DRAGONS_BREATH))
    {
        // Check if in frontal cone
        if (boss->HasInArc(M_PI / 2, bot) && !botAI->IsTank(bot))
        {
            // Calculate side escape direction
            float bossOrientation = boss->GetOrientation();
            float botAngleToBoss = boss->GetAngle(bot);
            
            // Choose left or right side
            float leftAngle = bossOrientation - (M_PI / 2);
            float rightAngle = bossOrientation + (M_PI / 2);
            
            // Choose closer side
            float angleDiffLeft = fabs(botAngleToBoss - leftAngle);
            float angleDiffRight = fabs(botAngleToBoss - rightAngle);
            
            float escapeAngle = (angleDiffLeft < angleDiffRight) ? leftAngle : rightAngle;
            
            // Move to safe position at boss side
            float escapeDistance = 15.0f;
            Position safePos;
            safePos.m_positionX = boss->GetPositionX() + cos(escapeAngle) * escapeDistance;
            safePos.m_positionY = boss->GetPositionY() + sin(escapeAngle) * escapeDistance;
            safePos.m_positionZ = boss->GetPositionZ();
            
            return MoveTo(bot->GetMapId(), safePos.m_positionX, safePos.m_positionY,
                         safePos.m_positionZ, false, false, false, true,
                         MovementPriority::MOVEMENT_NORMAL);
        }
    }

    return false;
}

bool SepethreaDragonsBreathAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* breathValue = botAI->GetAiObjectContext()->GetValue<bool>("dragons breath danger");
    if (!breathValue)
        return false;
    
    return breathValue->Get();
}


bool SepethreaArcaneBlastAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "nethermancer sepethrea");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Arcane Blast - threat reduction, tanks need to taunt back
    if (botAI->IsTank(bot))
    {
        if (boss->GetVictim() && boss->GetVictim() != bot)
        {
            // Check for Arcane Blast cast
            if (boss->HasUnitState(UNIT_STATE_CASTING) && 
                boss->FindCurrentSpellBySpellId(MECH_SPELL_ARCANE_BLAST))
            {
                // Taunt after cast
                return botAI->CastSpell("taunt", boss);
            }
        }
    }

    return false;
}

bool SepethreaArcaneBlastAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI || !botAI->IsTank(bot))
        return false;

    Value<bool>* engagedValue = botAI->GetAiObjectContext()->GetValue<bool>("sepethrea engaged");
    if (!engagedValue)
        return false;
    
    return engagedValue->Get();
}

bool SepethreaTargetElementalAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "nethermancer sepethrea");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // FORCE target back to boss (don't waste time on flames - focus burn boss)
    if (bot->GetSelectedUnit() != boss)
    {
        bot->SetSelection(boss->GetGUID());
        return true;
    }

    return false;
}

bool SepethreaTargetElementalAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || botAI->IsTank(bot) || botAI->IsHeal(bot))
        return false;

    // Check for Raging Flames targeting bot
    Unit* boss = AI_VALUE2(Unit*, "find target", "nethermancer sepethrea");
    if (!boss)
        return false;

    // Check for Raging Flames
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    bool hasFlameTargetingBot = false;
    
    for (auto& npc : npcs)
    {
        Unit* flame = botAI->GetUnit(npc);
        if (!flame || !flame->IsAlive())
            continue;
            
        if (flame->GetEntry() == NPC_RAGING_FLAMES && flame->GetVictim() == bot)
        {
            hasFlameTargetingBot = true;
            break;
        }
    }
    
    if (!hasFlameTargetingBot)
        return false;

    // Only trigger if not already targeting boss
    return bot->GetSelectedUnit() != boss;
}

// UNIVERSAL RAGING FLAMES AVOIDANCE - for ALL bots (not just targeted)
bool SepethreaAvoidRagingFlamesAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find nearest Raging Flames in the area
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* nearestFlame = nullptr;
    float closestDistance = 100.0f;
    
    for (auto& npc : npcs)
    {
        Unit* flame = botAI->GetUnit(npc);
        if (!flame || !flame->IsAlive() || flame->GetEntry() != NPC_RAGING_FLAMES)
            continue;
            
        float distance = bot->GetDistance(flame);
        if (distance < closestDistance)
        {
            nearestFlame = flame;
            closestDistance = distance;
        }
    }
    
    if (!nearestFlame)
        return false;

    const bool heroic = bot->GetMap()->IsHeroic();
    const float MIN_SAFE_DISTANCE = heroic ? 22.0f : 20.0f; // Larger bubble on heroic (3 flames)
    
    // If too close to ANY Raging Flames, move away (like ICC gas cloud)
    if (closestDistance < MIN_SAFE_DISTANCE)
    {
        // Calculate angle away from flame
        float angle = bot->GetAngle(nearestFlame);
        float x = bot->GetPositionX() + cos(angle) * -MIN_SAFE_DISTANCE;
        float y = bot->GetPositionY() + sin(angle) * -MIN_SAFE_DISTANCE;
        
        Position targetPos(x, y, bot->GetPositionZ(), 0.0f);
        targetPos = ConstrainToRoom(targetPos, botAI); // Keep in bounds
        
        if (IsPositionSafe(targetPos, botAI))
        {
            return MoveTo(bot->GetMapId(), targetPos.m_positionX, targetPos.m_positionY, 
                         targetPos.m_positionZ, false, false, false, true, 
                         MovementPriority::MOVEMENT_COMBAT);
        }
        
        // Fallback: simple flee with proper priority
        return FleePosition(nearestFlame->GetPosition(), heroic ? 22.0f : 18.0f, 1500U);
    }
    
    return false;
}

bool SepethreaAvoidRagingFlamesAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "nethermancer sepethrea");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if any Raging Flames are too close
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    for (auto& npc : npcs)
    {
        Unit* flame = botAI->GetUnit(npc);
        if (!flame || !flame->IsAlive() || flame->GetEntry() != NPC_RAGING_FLAMES)
            continue;
            
        if (bot->GetDistance(flame) < 16.0f) // Too close to area aura + inferno range
            return true;
    }
    
    return false;
}

bool SepethreaInfernoAvoidanceAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* infernoFlame = nullptr;
    float closestDistance = 20.0f;

    // Find closest Raging Flame casting Inferno
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive() || unit->GetEntry() != NPC_RAGING_FLAMES)
            continue;

        if (unit->FindCurrentSpellBySpellId(SPELL_INFERNO))
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                infernoFlame = unit;
                closestDistance = distance;
            }
        }
    }

    if (!infernoFlame)
        return false;

    // Flee from the flame casting inferno
    float fleeDistance = 22.0f;
    float angle = bot->GetAngle(infernoFlame) + M_PI;

    Position botPos = bot->GetPosition();
    Position targetPos;
    targetPos.m_positionX = botPos.m_positionX + cos(angle) * fleeDistance;
    targetPos.m_positionY = botPos.m_positionY + sin(angle) * fleeDistance;
    targetPos.m_positionZ = botPos.m_positionZ;

    Position safePos = ConstrainToRoom(targetPos, botAI);

    if (IsPathClear(botPos, safePos, botAI) && IsPathSafeFromFlames(botPos, safePos, botAI, 10.0f))
    {
        return MoveTo(bot->GetMapId(), safePos.m_positionX, safePos.m_positionY, 
                     safePos.m_positionZ, false, false, false, true, 
                     MovementPriority::MOVEMENT_NORMAL);
    }
    else
    {
        // If path is not clear, find a better escape route
        for (int i = 0; i < 8; ++i)
        {
            float current_angle = i * (M_PI / 4);
            Position checkPos;
            checkPos.m_positionX = botPos.m_positionX + cos(current_angle) * 15.0f;
            checkPos.m_positionY = botPos.m_positionY + sin(current_angle) * 15.0f;
            checkPos.m_positionZ = botPos.m_positionZ;

            if (IsPathClear(botPos, checkPos, botAI) && IsPathSafeFromFlames(botPos, checkPos, botAI, 10.0f))
            {
                Position newPos = ConstrainToRoom(checkPos);
                return MoveTo(bot->GetMapId(), newPos.m_positionX, newPos.m_positionY,
                             newPos.m_positionZ, false, false, false, true,
                             MovementPriority::MOVEMENT_NORMAL);
            }
        }
    }

    // Absolute fallback: move to center if no path found
    Position centerPos = MECHANAR_SEPETHREA_CENTER;
    return MoveTo(bot->GetMapId(), centerPos.m_positionX, centerPos.m_positionY,
                    centerPos.m_positionZ, false, false, false, true,
                    MovementPriority::MOVEMENT_FORCED);
}

bool SepethreaInfernoAvoidanceAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "nethermancer sepethrea");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive() || unit->GetEntry() != NPC_RAGING_FLAMES)
            continue;

        // Check for Inferno casting OR active Inferno aura (persistent damage)
        if ((unit->FindCurrentSpellBySpellId(SPELL_INFERNO) || unit->HasAura(SPELL_INFERNO)) 
            && bot->GetDistance(unit) < 25.0f)
            return true;
    }
    
    return false;
}

// Fire trail avoidance for all bots - ENHANCED VERSION
bool SepethreaFireTrailAvoidanceAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // IMMEDIATE emergency response to standing in fire trail
    if (bot->IsNonMeleeSpellCast(false))
    {
        // Only interrupt long casts or if taking heavy damage
        Spell* currentSpell = bot->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (currentSpell)
        {
            uint32 castTime = currentSpell->GetCastTime();
            float healthPct = bot->GetHealthPct();
            
            // Interrupt if: long cast (>2s) or health dropping quickly (<60%)
            if (castTime > 2000 || healthPct < 60.0f)
            {
                botAI->InterruptSpell();
            }
        }
        else
        {
            // Fallback - interrupt if no current spell info available
            botAI->InterruptSpell();
        }
    }
    
    // ENHANCED FIRE ESCAPE: Use FleePosition instead of complex manual pathfinding
    // FleePosition automatically handles room boundaries and collision detection
    
    // Find all nearby Raging Flames to flee from
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* nearestFlame = nullptr;
    float closestDistance = 100.0f;
    
    for (auto& npc : npcs)
    {
        Unit* flame = botAI->GetUnit(npc);
        if (!flame || !flame->IsAlive() || flame->GetEntry() != NPC_RAGING_FLAMES)
            continue;
            
        float distance = bot->GetDistance(flame);
        if (distance < closestDistance)
        {
            nearestFlame = flame;
            closestDistance = distance;
        }
    }
    
    if (nearestFlame)
    {
        // SMART ESCAPE: Flee from nearest flame with emergency priority
        float escapeDistance = 18.0f; // Increased from area aura range
        uint32 minTime = 1000; // Quick escape time
        
        // Try a few angular offsets to avoid fleeing through a flame path
        Position from = bot->GetPosition();
        for (int i = 0; i < 8; ++i)
        {
            float ang = bot->GetAngle(nearestFlame) + M_PI + (i % 2 == 0 ? 1 : -1) * (i * 0.2f);
            Position candidate;
            candidate.m_positionX = from.m_positionX + cosf(ang) * escapeDistance;
            candidate.m_positionY = from.m_positionY + sinf(ang) * escapeDistance;
            candidate.m_positionZ = from.m_positionZ;
            candidate = ConstrainToRoom(candidate, botAI);
            if (IsPathClear(from, candidate, botAI) && IsPathSafeFromFlames(from, candidate, botAI, 10.0f))
            {
                return MoveTo(bot->GetMapId(), candidate.m_positionX, candidate.m_positionY,
                              candidate.m_positionZ, false, false, false, true,
                              MovementPriority::MOVEMENT_COMBAT);
            }
        }
        return FleePosition(nearestFlame->GetPosition(), escapeDistance, minTime);
    }
    
    // FALLBACK: If no flames found but still in fire aura, move to room center
    if (bot->HasAura(SPELL_RAGING_FLAMES_AREA_AURA))
    {
        Position centerPos = MECHANAR_SEPETHREA_CENTER;
        return MoveTo(bot->GetMapId(), centerPos.m_positionX, centerPos.m_positionY,
                        centerPos.m_positionZ, false, false, false, true,
                        MovementPriority::MOVEMENT_FORCED);
    }
    
    return false;
}

bool SepethreaFireTrailAvoidanceAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "nethermancer sepethrea");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if standing in fire trail aura or near any flame within 8 yards
    if (bot->HasAura(SPELL_RAGING_FLAMES_AREA_AURA) || bot->HasAura(SPELL_INFERNO_DAMAGE))
        return true;

    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* flame = botAI->GetUnit(npc);
        if (!flame || !flame->IsAlive() || flame->GetEntry() != NPC_RAGING_FLAMES)
            continue;
        if (bot->GetDistance(flame) < 12.0f)
            return true;
    }
    return false;
}

// ========== PATHALEON THE CALCULATOR ACTIONS ==========

bool PathaleonDominationAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Domination is mind control - affected players need to be CC'd
    const GuidVector members = AI_VALUE(GuidVector, "group members");
    
    for (auto& member : members)
    {
        Unit* ally = botAI->GetUnit(member);
        if (!ally || ally == bot || !ally->IsAlive())
            continue;

        if (ally->HasAura(MECH_SPELL_DOMINATION))
        {
            // CC the mind controlled ally
            if (bot->getClass() == CLASS_MAGE)
            {
                return botAI->CastSpell("polymorph", ally);
            }
            else if (bot->getClass() == CLASS_PRIEST)
            {
                return botAI->CastSpell("shackle undead", ally);
            }
            else if (bot->getClass() == CLASS_WARLOCK)
            {
                return botAI->CastSpell("fear", ally);
            }
        }
    }

    // If we are dominated, can't do anything
    if (bot->HasAura(MECH_SPELL_DOMINATION))
    {
        return true;
    }

    return false;
}

bool PathaleonDominationAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* dominationValue = botAI->GetAiObjectContext()->GetValue<bool>("domination active");
    if (!dominationValue)
        return false;
    
    return dominationValue->Get();
}

bool PathaleonNetherWraithAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Summons 3-4 Nether Wraiths that need to be killed quickly
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* nearestWraith = nullptr;
    float closestDistance = 100.0f;

    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_NETHER_WRAITH)
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                nearestWraith = unit;
                closestDistance = distance;
            }
        }
    }

    if (nearestWraith)
    {
        return Attack(nearestWraith);
    }

    return false;
}

bool PathaleonNetherWraithAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* wraithValue = botAI->GetAiObjectContext()->GetValue<bool>("nether wraith active");
    if (!wraithValue)
        return false;
    
    return wraithValue->Get();
}

bool PathaleonArcaneTorrentAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "pathaleon the calculator");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Arcane Torrent is an AoE around the boss
    if (boss->HasUnitState(UNIT_STATE_CASTING) && 
        boss->FindCurrentSpellBySpellId(MECH_SPELL_ARCANE_TORRENT))
    {
        float distance = bot->GetDistance(boss);
        if (distance < 20.0f && !botAI->IsMelee(bot))
        {
            // Move out of Arcane Torrent range
            float angle = bot->GetAngle(boss) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 25.0f;
            float y = bot->GetPositionY() + sin(angle) * 25.0f;
            float z = bot->GetPositionZ();
            Position targetPos(x, y, z, 0.0f);
            targetPos = ConstrainToRoom(targetPos, botAI);
            
            if (IsPositionSafe(targetPos, botAI) && IsPathClear(bot->GetPosition(), targetPos, botAI))
            {
                return MoveTo(bot->GetMapId(), targetPos.m_positionX, targetPos.m_positionY,
                             targetPos.m_positionZ, false, false, false, true,
                             MovementPriority::MOVEMENT_NORMAL);
            }
            
            // Fallback: find safe direction
            for (int i = 0; i < 8; ++i)
            {
                float testAngle = (i * M_PI / 4);
                float testX = bot->GetPositionX() + cos(testAngle) * 8.0f;
                float testY = bot->GetPositionY() + sin(testAngle) * 8.0f;
                Position testPos(testX, testY, z, 0.0f);
                testPos = ConstrainToRoom(testPos, botAI);
                
                if (IsPositionSafe(testPos, botAI) && IsPathClear(bot->GetPosition(), testPos, botAI))
                {
                    return MoveTo(bot->GetMapId(), testPos.m_positionX, testPos.m_positionY,
                                 testPos.m_positionZ, false, false, false, true,
                                 MovementPriority::MOVEMENT_NORMAL);
                }
            }
        }
    }

    return false;
}

bool PathaleonArcaneTorrentAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* torrentValue = botAI->GetAiObjectContext()->GetValue<bool>("arcane torrent danger");
    if (!torrentValue)
        return false;
    
    return torrentValue->Get();
}

bool PathaleonEnrageAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "pathaleon the calculator");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // At 20% health: Frenzy + Suicide (explodes on death)
    if (boss->HasAura(MECH_SPELL_FRENZY))
    {
        // Boss will explode on death - prepare to run
        if (boss->GetHealthPct() < 5.0f)
        {
            // Get ready to flee when he dies (suicide explosion)
            float angle = bot->GetAngle(boss) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 30.0f;
            float y = bot->GetPositionY() + sin(angle) * 30.0f;
            float z = bot->GetPositionZ();
            Position targetPos(x, y, z, 0.0f);
            targetPos = ConstrainToRoom(targetPos, botAI);
            
            if (IsPositionSafe(targetPos, botAI) && IsPathClear(bot->GetPosition(), targetPos, botAI))
            {
                return MoveTo(bot->GetMapId(), targetPos.m_positionX, targetPos.m_positionY,
                             targetPos.m_positionZ, false, false, false, true,
                             MovementPriority::MOVEMENT_NORMAL);
            }
            
            // Fallback: find safe direction
            for (int i = 0; i < 8; ++i)
            {
                float testAngle = (i * M_PI / 4);
                float testX = bot->GetPositionX() + cos(testAngle) * 8.0f;
                float testY = bot->GetPositionY() + sin(testAngle) * 8.0f;
                Position testPos(testX, testY, z, 0.0f);
                testPos = ConstrainToRoom(testPos, botAI);
                
                if (IsPositionSafe(testPos, botAI) && IsPathClear(bot->GetPosition(), testPos, botAI))
                {
                    return MoveTo(bot->GetMapId(), testPos.m_positionX, testPos.m_positionY,
                                 testPos.m_positionZ, false, false, false, true,
                                 MovementPriority::MOVEMENT_NORMAL);
                }
            }
        }
        
        // Healers focus on tank during enrage
        if (botAI->IsHeal(bot))
        {
            const GuidVector members = AI_VALUE(GuidVector, "group members");
            for (auto& member : members)
            {
                Unit* ally = botAI->GetUnit(member);
                if (ally && ally->IsAlive() && ally->IsPlayer() && botAI->IsTank(ally->ToPlayer()))
                {
                    if (ally->GetHealthPct() < 70.0f)
                    {
                        return botAI->CastSpell("heal", ally);
                    }
                }
            }
        }
        
        // DPS burn hard
        if (!botAI->IsTank(bot) && !botAI->IsHeal(bot))
        {
            return botAI->CastSpell("boost", bot);
        }
    }

    return false;
}

bool PathaleonEnrageAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "pathaleon the calculator");
    return boss && boss->HasAura(MECH_SPELL_FRENZY);
}

bool PathaleonArcaneExplosionAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot || !bot->GetMap()->IsHeroic())
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "pathaleon the calculator");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Arcane Explosion on Heroic - AoE around boss
    if (boss->HasUnitState(UNIT_STATE_CASTING) && 
        boss->FindCurrentSpellBySpellId(MECH_SPELL_ARCANE_EXPLOSION))
    {
        float distance = bot->GetDistance(boss);
        if (distance < 10.0f && !botAI->IsMelee(bot))
        {
            // Move out of Arcane Explosion range
            float angle = bot->GetAngle(boss) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 15.0f;
            float y = bot->GetPositionY() + sin(angle) * 15.0f;
            float z = bot->GetPositionZ();
            Position targetPos(x, y, z, 0.0f);
            targetPos = ConstrainToRoom(targetPos, botAI);
            
            if (IsPositionSafe(targetPos, botAI) && IsPathClear(bot->GetPosition(), targetPos, botAI))
            {
                return MoveTo(bot->GetMapId(), targetPos.m_positionX, targetPos.m_positionY,
                             targetPos.m_positionZ, false, false, false, true,
                             MovementPriority::MOVEMENT_NORMAL);
            }
            
            // Fallback: find safe direction
            for (int i = 0; i < 8; ++i)
            {
                float testAngle = (i * M_PI / 4);
                float testX = bot->GetPositionX() + cos(testAngle) * 8.0f;
                float testY = bot->GetPositionY() + sin(testAngle) * 8.0f;
                Position testPos(testX, testY, z, 0.0f);
                testPos = ConstrainToRoom(testPos, botAI);
                
                if (IsPositionSafe(testPos, botAI) && IsPathClear(bot->GetPosition(), testPos, botAI))
                {
                    return MoveTo(bot->GetMapId(), testPos.m_positionX, testPos.m_positionY,
                                 testPos.m_positionZ, false, false, false, true,
                                 MovementPriority::MOVEMENT_NORMAL);
                }
            }
        }
    }

    return false;
}

bool PathaleonArcaneExplosionAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI || !bot->GetMap()->IsHeroic())
        return false;

    Value<bool>* explosionValue = botAI->GetAiObjectContext()->GetValue<bool>("arcane explosion danger");
    if (!explosionValue)
        return false;
    
    return explosionValue->Get();
}

bool PathaleonManaTapAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Mana Tap drains mana from a random player
    if (bot->HasAura(MECH_SPELL_MANA_TAP))
    {
        // Affected player should use mana efficiently
        // Healers prioritize emergency heals only
        if (botAI->IsHeal(bot))
        {
            const GuidVector members = AI_VALUE(GuidVector, "group members");
            for (auto& member : members)
            {
                Unit* ally = botAI->GetUnit(member);
                if (ally && ally->IsAlive() && ally->GetHealthPct() < 30.0f)
                {
                    return botAI->CastSpell("heal", ally);
                }
            }
        }
    }

    return false;
}

bool PathaleonManaTapAction::isUseful()
{
    Player* bot = botAI->GetBot();
    return bot && bot->getPowerType() == POWER_MANA && bot->HasAura(MECH_SPELL_MANA_TAP);
}
