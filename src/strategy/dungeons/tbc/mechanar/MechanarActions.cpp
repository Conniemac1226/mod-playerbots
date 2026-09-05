#include "MechanarActions.h"
#include "MechanarTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "VMapFactory.h"

using namespace VMAP;

// Sepethrea kiting state
static std::map<ObjectGuid, uint32> g_sepethrea_lastKiteMove;

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

bool IsPositionSafe(const Position& pos, PlayerbotAI* botAI);
bool IsPathClear(const Position& from, const Position& to, PlayerbotAI* botAI);
bool IsPathSafeFromFlames(const Position& from, const Position& to, PlayerbotAI* botAI, float minDistance);
Position ConstrainToRoom(const Position& pos, PlayerbotAI* botAI);

namespace
{
bool UseAvailableTankTaunt(PlayerbotAI* botAI, Player* bot, Unit* target, Event event)
{
    if (!botAI || !bot || !target)
        return false;

    switch (bot->getClass())
    {
        case CLASS_PALADIN:
            return botAI->DoSpecificAction("hand of reckoning", event, true);
        case CLASS_DRUID:
            return botAI->DoSpecificAction("growl", event, true);
        case CLASS_DEATH_KNIGHT:
            return botAI->DoSpecificAction("dark command", event, true);
        case CLASS_WARRIOR:
            return botAI->DoSpecificAction("taunt", event, true);
        default:
            return botAI->CastSpell("taunt", target);
    }
}

float GetMinimumFlameDistance(PlayerbotAI* botAI, Position const& candidate)
{
    if (!botAI)
        return 1000.0f;

    float minFlameDistance = 1000.0f;
    GuidVector const npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
    for (ObjectGuid const& guid : npcs)
    {
        Unit* flame = botAI->GetUnit(guid);
        if (!flame || !flame->IsAlive() || flame->GetEntry() != NPC_RAGING_FLAMES)
            continue;

        minFlameDistance = std::min(minFlameDistance, candidate.GetExactDist2d(flame));
    }

    return minFlameDistance;
}

std::vector<Unit*> GetActiveRagingFlames(PlayerbotAI* botAI)
{
    std::vector<Unit*> flames;
    if (!botAI)
        return flames;

    GuidVector const npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
    flames.reserve(8);
    for (ObjectGuid const& guid : npcs)
    {
        Unit* flame = botAI->GetUnit(guid);
        if (flame && flame->IsAlive() && flame->GetEntry() == NPC_RAGING_FLAMES)
            flames.push_back(flame);
    }

    return flames;
}

bool SelectSepethreaEscapePosition(PlayerbotAI* botAI, Player* bot, float preferredDistance,
                                   float minimumFlameDistance, Position& bestPos,
                                   Unit* priorityFlame = nullptr)
{
    if (!botAI || !bot)
        return false;

    std::vector<Unit*> flames = GetActiveRagingFlames(botAI);
    if (flames.empty())
        return false;

    Position const from = bot->GetPosition();
    Position const center = MECHANAR_SEPETHREA_CENTER;
    float bestScore = -100000.0f;

    auto scoreCandidate = [&](Position const& candidate) -> float
    {
        float minDist = 1000.0f;
        float totalDist = 0.0f;
        float infernoBonus = 0.0f;

        for (Unit* flame : flames)
        {
            float dist = candidate.GetExactDist2d(flame);
            minDist = std::min(minDist, dist);
            totalDist += dist;

            if (flame->FindCurrentSpellBySpellId(SPELL_INFERNO) || flame->HasAura(SPELL_INFERNO))
                infernoBonus += std::min(dist, 24.0f) * 0.6f;
        }

        if (minDist < minimumFlameDistance)
            return -100000.0f;

        float score = minDist * 3.0f + totalDist * 0.25f + infernoBonus;

        float centerDist = candidate.GetExactDist2d(&center);
        if (centerDist < 10.0f)
            score -= (10.0f - centerDist) * 1.5f;
        if (centerDist > 24.0f)
            score -= (centerDist - 24.0f) * 1.2f;

        if (priorityFlame)
            score += std::min(candidate.GetExactDist2d(priorityFlame), 30.0f) * 1.5f;

        if (botAI->IsHeal(bot))
        {
            float closestTank = 1000.0f;
            GuidVector const members = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members")->Get();
            for (ObjectGuid const& memberGuid : members)
            {
                Player* member = botAI->GetPlayer(memberGuid);
                if (member && member->IsAlive() && botAI->IsTank(member))
                    closestTank = std::min(closestTank, candidate.GetExactDist2d(member));
            }

            if (closestTank > 34.0f)
                score -= (closestTank - 34.0f) * 1.4f;
        }

        return score;
    };

    float baseAngle = atan2f(from.GetPositionY() - center.GetPositionY(),
                             from.GetPositionX() - center.GetPositionX());
    std::vector<float> angles = { baseAngle, baseAngle + 0.45f, baseAngle - 0.45f, baseAngle + 0.9f,
                                  baseAngle - 0.9f, baseAngle + 1.35f, baseAngle - 1.35f,
                                  baseAngle + static_cast<float>(M_PI), baseAngle + 2.2f,
                                  baseAngle - 2.2f };

    if (priorityFlame)
    {
        float away = bot->GetAngle(priorityFlame) + static_cast<float>(M_PI);
        angles.insert(angles.begin(), away);
        angles.insert(angles.begin() + 1, away + 0.65f);
        angles.insert(angles.begin() + 2, away - 0.65f);
    }

    for (float angle : angles)
    {
        for (float dist = preferredDistance; dist >= 10.0f; dist -= 2.0f)
        {
            Position candidate;
            candidate.m_positionX = from.GetPositionX() + cosf(angle) * dist;
            candidate.m_positionY = from.GetPositionY() + sinf(angle) * dist;
            candidate.m_positionZ = from.GetPositionZ();
            candidate = ConstrainToRoom(candidate, botAI);

            if (!IsPositionSafe(candidate, botAI))
                continue;
            if (!IsPathClear(from, candidate, botAI))
                continue;
            if (!IsPathSafeFromFlames(from, candidate, botAI, minimumFlameDistance - 2.0f))
                continue;

            float score = scoreCandidate(candidate);
            if (score > bestScore)
            {
                bestScore = score;
                bestPos = candidate;
            }
        }
    }

    return bestScore > -1000.0f;
}
}

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
            botAI->GetBot()->InterruptNonMeleeSpells(true);
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

    bool hasPositive = bot->HasAura(SPELL_POSITIVE_POLARITY);
    bool hasNegative = bot->HasAura(SPELL_NEGATIVE_POLARITY);

    if (!hasPositive && !hasNegative)
        return false;

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

    bool const heroic = bot->GetMap()->IsHeroic();
    bool const isHealer = botAI->IsHeal(bot);
    float const flameDistance = bot->GetDistance(targetingFlame);
    float const minFlameDistance = GetMinimumFlameDistance(botAI, bot->GetPosition());
    bool const urgent =
        flameDistance < (heroic ? 16.0f : 14.0f) || minFlameDistance < (heroic ? 14.0f : 12.0f);

    // Kite immediately once fixated; the move throttle below prevents path spam.
    if (bot->IsNonMeleeSpellCast(false))
    {
        if (Spell* currentSpell = bot->GetCurrentSpell(CURRENT_GENERIC_SPELL))
        {
            uint32 castTime = currentSpell->GetCastTime();
            if (urgent || castTime > 1500 || flameDistance < (heroic ? 18.0f : 15.0f))
                botAI->GetBot()->InterruptNonMeleeSpells(true);
        }
        else
        {
            botAI->GetBot()->InterruptNonMeleeSpells(true);
        }
    }

    ObjectGuid guid = bot->GetGUID();
    uint32 now = getMSTime();
    uint32& lastMove = g_sepethrea_lastKiteMove[guid];
    if (!urgent && lastMove && now - lastMove < 650)
        return false;

    Position escapePos;
    float const preferredDistance = heroic ? (isHealer ? 24.0f : 22.0f) : (isHealer ? 21.0f : 19.0f);
    float const requiredFlameDistance = heroic ? (urgent ? 17.0f : 15.0f) : (urgent ? 14.0f : 12.0f);
    auto const moveForced = [this, bot](Position const& pos)
    {
        return MoveTo(bot->GetMapId(), pos.m_positionX, pos.m_positionY, pos.m_positionZ,
                      false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    };

    if (SelectSepethreaEscapePosition(botAI, bot, preferredDistance, requiredFlameDistance, escapePos, targetingFlame) ||
        SelectSepethreaEscapePosition(botAI, bot, preferredDistance - 3.0f, requiredFlameDistance - 4.0f, escapePos, targetingFlame))
    {
        if (moveForced(escapePos))
        {
            lastMove = now;
            return true;
        }
    }

    Position away;
    float awayAngle = bot->GetAngle(targetingFlame) + M_PI;
    float awayDist = heroic ? (isHealer ? 22.0f : 19.0f) : (isHealer ? 19.0f : 16.0f);
    away.m_positionX = bot->GetPositionX() + cosf(awayAngle) * awayDist;
    away.m_positionY = bot->GetPositionY() + sinf(awayAngle) * awayDist;
    away.m_positionZ = bot->GetPositionZ();
    Position safeAway = ConstrainToRoom(away, botAI);

    if (IsPositionSafe(safeAway, botAI) && IsPathClear(bot->GetPosition(), safeAway, botAI) &&
        IsPathSafeFromFlames(bot->GetPosition(), safeAway, botAI, requiredFlameDistance - 2.0f) &&
        moveForced(safeAway))
    {
        lastMove = now;
        return true;
    }

    return false;
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
        if (bot->IsNonMeleeSpellCast(false))
            botAI->GetBot()->InterruptNonMeleeSpells(true);

        auto const moveForced = [this, bot](Position const& pos)
        {
            return MoveTo(bot->GetMapId(), pos.m_positionX, pos.m_positionY, pos.m_positionZ,
                          false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        };

        auto const forceFlee = [this, bot, &moveForced](Unit* source, float radius)
        {
            Position fleePos = botAI->IsMelee(bot) ? BestPositionForMeleeToFlee(source->GetPosition(), radius) :
                                                     BestPositionForRangedToFlee(source->GetPosition(), radius);
            if (fleePos == Position())
                return false;

            fleePos = ConstrainToRoom(fleePos, botAI);
            if (!IsPositionSafe(fleePos, botAI) || !IsPathClear(bot->GetPosition(), fleePos, botAI) ||
                !IsPathSafeFromFlames(bot->GetPosition(), fleePos, botAI, 10.0f))
            {
                return false;
            }

            return moveForced(fleePos);
        };

        // Check if in frontal cone
        if (boss->HasInArc(M_PI / 2, bot) && !botAI->IsTank(bot))
        {
            float bossOrientation = boss->GetOrientation();
            float const halfPi = static_cast<float>(M_PI / 2);
            float const sideAngles[2] = { bossOrientation - halfPi, bossOrientation + halfPi };
            float bestScore = -1.0f;
            Position bestPos;

            for (float sideAngle : sideAngles)
            {
                for (float escapeDistance : {12.0f, 15.0f, 18.0f})
                {
                    Position safePos;
                    safePos.m_positionX = boss->GetPositionX() + cosf(sideAngle) * escapeDistance;
                    safePos.m_positionY = boss->GetPositionY() + sinf(sideAngle) * escapeDistance;
                    safePos.m_positionZ = bot->GetPositionZ();
                    safePos = ConstrainToRoom(safePos, botAI);

                    if (!IsPositionSafe(safePos, botAI))
                        continue;
                    if (!IsPathClear(bot->GetPosition(), safePos, botAI))
                        continue;
                    if (!IsPathSafeFromFlames(bot->GetPosition(), safePos, botAI, 10.0f))
                        continue;

                    float score = GetMinimumFlameDistance(botAI, safePos);
                    if (score > bestScore)
                    {
                        bestScore = score;
                        bestPos = safePos;
                    }
                }
            }

            if (bestScore > 0.0f)
            {
                if (moveForced(bestPos))
                    return true;
            }

            return forceFlee(boss, 14.0f);
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
        Unit* victim = boss->GetVictim();
        if (victim && victim != bot)
        {
            if (boss->HasUnitState(UNIT_STATE_CASTING) &&
                boss->FindCurrentSpellBySpellId(MECH_SPELL_ARCANE_BLAST))
                return false;

            return UseAvailableTankTaunt(botAI, bot, boss, event);
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

// Avoid nearby flames for bots that are not currently fixated.
bool SepethreaAvoidRagingFlamesAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    std::vector<Unit*> flames = GetActiveRagingFlames(botAI);
    if (flames.empty())
        return false;

    for (Unit* flame : flames)
    {
        if (flame->GetVictim() == bot)
            return false;
    }

    const bool heroic = bot->GetMap()->IsHeroic();
    float const minimumFlameDistance = heroic ? 18.0f : 15.0f;
    Unit* nearestFlame = nullptr;
    float closestDistance = 100.0f;

    for (Unit* flame : flames)
    {
        float distance = bot->GetDistance(flame);
        if (distance < closestDistance)
        {
            nearestFlame = flame;
            closestDistance = distance;
        }
    }

    if (!nearestFlame || closestDistance >= minimumFlameDistance)
        return false;

    if (bot->IsNonMeleeSpellCast(false))
        botAI->GetBot()->InterruptNonMeleeSpells(true);

    auto const moveForced = [this, bot](Position const& pos)
    {
        return MoveTo(bot->GetMapId(), pos.m_positionX, pos.m_positionY, pos.m_positionZ,
                      false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    };

    auto const forceFlee = [this, bot, &moveForced](Unit* source, float radius, float minDistance)
    {
        Position fleePos = botAI->IsMelee(bot) ? BestPositionForMeleeToFlee(source->GetPosition(), radius) :
                                                 BestPositionForRangedToFlee(source->GetPosition(), radius);
        if (fleePos == Position())
            return false;

        fleePos = ConstrainToRoom(fleePos, botAI);
        if (!IsPositionSafe(fleePos, botAI) || !IsPathClear(bot->GetPosition(), fleePos, botAI) ||
            !IsPathSafeFromFlames(bot->GetPosition(), fleePos, botAI, minDistance))
        {
            return false;
        }

        return moveForced(fleePos);
    };

    Position escapePos;
    if (SelectSepethreaEscapePosition(botAI, bot, heroic ? 20.0f : 17.0f,
                                      heroic ? 14.0f : 12.0f, escapePos, nearestFlame))
    {
        if (moveForced(escapePos))
            return true;
    }

    return forceFlee(nearestFlame, heroic ? 22.0f : 18.0f, heroic ? 14.0f : 12.0f);
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

        if (flame->GetVictim() == bot)
            return false;

        if (bot->GetDistance(flame) < (bot->GetMap()->IsHeroic() ? 18.0f : 16.0f))
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
    bool const heroic = bot->GetMap()->IsHeroic();
    float closestDistance = heroic ? 24.0f : 20.0f;

    // Find closest Raging Flame casting Inferno
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive() || unit->GetEntry() != NPC_RAGING_FLAMES)
            continue;

        if (unit->FindCurrentSpellBySpellId(SPELL_INFERNO) || unit->HasAura(SPELL_INFERNO))
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

    if (bot->IsNonMeleeSpellCast(false))
        botAI->GetBot()->InterruptNonMeleeSpells(true);

    auto const moveForced = [this, bot](Position const& pos)
    {
        return MoveTo(bot->GetMapId(), pos.m_positionX, pos.m_positionY, pos.m_positionZ,
                      false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    };

    auto const forceFlee = [this, bot, &moveForced](Unit* source, float radius, float minDistance)
    {
        Position fleePos = botAI->IsMelee(bot) ? BestPositionForMeleeToFlee(source->GetPosition(), radius) :
                                                 BestPositionForRangedToFlee(source->GetPosition(), radius);
        if (fleePos == Position())
            return false;

        fleePos = ConstrainToRoom(fleePos, botAI);
        if (!IsPositionSafe(fleePos, botAI) || !IsPathClear(bot->GetPosition(), fleePos, botAI) ||
            !IsPathSafeFromFlames(bot->GetPosition(), fleePos, botAI, minDistance))
        {
            return false;
        }

        return moveForced(fleePos);
    };

    Position safePos;
    if (SelectSepethreaEscapePosition(botAI, bot, heroic ? 22.0f : 19.0f,
                                      heroic ? 16.0f : 13.0f, safePos, infernoFlame))
    {
        if (moveForced(safePos))
            return true;
    }

    return forceFlee(infernoFlame, 22.0f, heroic ? 16.0f : 13.0f);
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
            && bot->GetDistance(unit) < (bot->GetMap()->IsHeroic() ? 24.0f : 20.0f))
            return true;
    }
    
    return false;
}

// Fire trail avoidance for all bots.
bool SepethreaFireTrailAvoidanceAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // IMMEDIATE emergency response to standing near the fire trail
    if (bot->IsNonMeleeSpellCast(false))
    {
        botAI->GetBot()->InterruptNonMeleeSpells(true);
    }
    
    std::vector<Unit*> flames = GetActiveRagingFlames(botAI);
    Unit* nearestFlame = nullptr;
    float closestDistance = 100.0f;
    for (Unit* flame : flames)
    {
        float distance = bot->GetDistance(flame);
        if (distance < closestDistance)
        {
            nearestFlame = flame;
            closestDistance = distance;
        }
    }

    if (nearestFlame)
    {
        Position safePos;
        float preferredDistance = bot->GetMap()->IsHeroic() ? 20.0f : 17.0f;
        float minimumFlameDistance = bot->GetMap()->IsHeroic() ? 14.0f : 12.0f;
        if (SelectSepethreaEscapePosition(botAI, bot, preferredDistance, minimumFlameDistance,
                                          safePos, nearestFlame))
        {
            return MoveTo(bot->GetMapId(), safePos.m_positionX, safePos.m_positionY,
                          safePos.m_positionZ, false, false, false, true,
                          MovementPriority::MOVEMENT_FORCED);
        }

        Position fleePos = botAI->IsMelee(bot) ? BestPositionForMeleeToFlee(nearestFlame->GetPosition(),
                                                                            preferredDistance)
                                               : BestPositionForRangedToFlee(nearestFlame->GetPosition(),
                                                                             preferredDistance);
        if (fleePos != Position())
        {
            fleePos = ConstrainToRoom(fleePos, botAI);
            if (IsPositionSafe(fleePos, botAI) && IsPathClear(bot->GetPosition(), fleePos, botAI) &&
                IsPathSafeFromFlames(bot->GetPosition(), fleePos, botAI, minimumFlameDistance))
            {
                return MoveTo(bot->GetMapId(), fleePos.m_positionX, fleePos.m_positionY,
                              fleePos.m_positionZ, false, false, false, true,
                              MovementPriority::MOVEMENT_FORCED);
            }
        }
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

    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* flame = botAI->GetUnit(npc);
        if (!flame || !flame->IsAlive() || flame->GetEntry() != NPC_RAGING_FLAMES)
            continue;

        float const dangerDistance = bot->GetMap()->IsHeroic() ? 18.0f : 16.0f;
        if (bot->GetDistance(flame) < dangerDistance)
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
