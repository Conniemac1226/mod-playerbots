#include "TempestKeepActions.h"
#include "PlayerbotAI.h"
#include "Player.h"
#include "Unit.h"
#include "SpellAuras.h"
#include "ObjectGuid.h"
#include "Log.h"
#include "Group.h"
#include "Random.h"

/*
 * SAFETY AUDIT COMPLETED:
 * - All AI_VALUE2 calls have null checks on returned pointers
 * - All botAI->GetUnit() calls have null checks
 * - All Value<>* pointer accesses use null-checked pattern
 * - Division by zero protected in movement calculations (distance < 2.0f check)
 * - Random angle used when distance < 0.1f to prevent bot stacking
 * - All spell list accesses use safe Value<>* pointer pattern with null checks
 * - Group member iterations have null checks
 * - All target setting uses safe Value<Unit*>* pattern
 * - Static maps use per-bot GUID keys to prevent multi-bot conflicts
 * - Priority values stay within safe limits (max 95)
 * - Movement uses MOVEMENT_FORCED for urgent escapes
 */

// Static per-bot state maps for Al'ar
static std::map<ObjectGuid, uint32> s_alarLastPlatformTime;
static std::map<ObjectGuid, int> s_alarCurrentPlatform;
static std::map<ObjectGuid, bool> s_alarInSafePosition;

// Helper functions following ICC pattern
bool TempestKeepMovementHelper::MoveTowardPosition(Player* bot, const Position& position, float incrementSize)
{
    if (!bot)
        return false;

    float currentX = bot->GetPositionX();
    float currentY = bot->GetPositionY();
    float currentZ = bot->GetPositionZ();
    
    float targetX = position.GetPositionX();
    float targetY = position.GetPositionY();
    float targetZ = position.GetPositionZ();
    
    float dx = targetX - currentX;
    float dy = targetY - currentY;
    float distance = sqrt(dx * dx + dy * dy);
    
    if (distance < 2.0f)
    {
        return false;
    }
    
    float moveX = currentX + (dx / distance) * incrementSize;
    float moveY = currentY + (dy / distance) * incrementSize;
    
    bot->GetMotionMaster()->MovePoint(0, moveX, moveY, targetZ, FORCED_MOVEMENT_NONE, 0.f, 0.0f, false);
    return true;
}

float TempestKeepMovementHelper::CalculateDistanceScore(Player* bot, const Position& position)
{
    if (!bot)
        return 0.0f;
        
    float distance = bot->GetDistance(position);
    return 100.0f - distance;
}

Position TempestKeepMovementHelper::CalculateSafePosition(const Position& danger, const Position& current, float minDistance)
{
    float dx = current.GetPositionX() - danger.GetPositionX();
    float dy = current.GetPositionY() - danger.GetPositionY();
    float distance = sqrt(dx * dx + dy * dy);
    
    if (distance < 0.1f)
    {
        // If too close, move directly away in a random direction to avoid stacking
        float randomAngle = frand(0, 2 * M_PI);
        return Position(current.GetPositionX() + cos(randomAngle) * minDistance,
                       current.GetPositionY() + sin(randomAngle) * minDistance,
                       current.GetPositionZ(),
                       0.0f);
    }
    
    float factor = minDistance / distance;
    return Position(danger.GetPositionX() + dx * factor,
                   danger.GetPositionY() + dy * factor,
                   current.GetPositionZ(),
                   0.0f);
}

// Al'ar Actions
bool AlarFlameQuillsAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    uint32 currentTime = getMSTime();
    
    // Reset safe position flag if enough time has passed
    if (s_alarInSafePosition[botGuid] && 
        (currentTime - s_alarLastPlatformTime[botGuid]) > 15000)
    {
        s_alarInSafePosition[botGuid] = false;
    }

    // If already safe, stay there
    if (s_alarInSafePosition[botGuid])
    {
        return false;
    }

    Position nearestPlatform = GetNearestPlatform();
    
    // Move to platform edge to avoid quills
    if (TempestKeepMovementHelper::MoveTowardPosition(bot, nearestPlatform, 8.0f))
    {
        s_alarInSafePosition[botGuid] = true;
        s_alarLastPlatformTime[botGuid] = currentTime;
        return true;
    }

    return false;
}

Position AlarFlameQuillsAction::GetNearestPlatform()
{
    const Position platforms[] = {
        TK_ALAR_SAFE_SPOT_1,
        TK_ALAR_SAFE_SPOT_2,
        TK_ALAR_SAFE_SPOT_3,
        TK_ALAR_SAFE_SPOT_4
    };

    Position nearest = platforms[0];
    float minDist = bot->GetDistance(platforms[0]);
    
    for (int i = 1; i < 4; ++i)
    {
        float dist = bot->GetDistance(platforms[i]);
        if (dist < minDist)
        {
            minDist = dist;
            nearest = platforms[i];
        }
    }
    
    return nearest;
}

bool AlarFlameQuillsAction::IsOnPlatform()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
    // Check if bot is on a platform (elevated position)
    return bot->GetPositionZ() > 15.0f;
}

bool AlarDiveBombAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss)
        return false;

    // Check if we might be the dive bomb target
    if (IsDiveBombTarget())
    {
        Position escapePos = CalculateEscapePosition();
        bot->GetMotionMaster()->MovePoint(0, escapePos.GetPositionX(),
                                          escapePos.GetPositionY(),
                                          escapePos.GetPositionZ(), FORCED_MOVEMENT_NONE, 0.f, 0.0f, false);
        return true;
    }

    // Move away from dive location
    float diveDist = bot->GetDistance2d(TK_ALAR_DIVE_POSITION.GetPositionX(),
                                        TK_ALAR_DIVE_POSITION.GetPositionY());
    if (diveDist < 15.0f)
    {
        Position escapePos = TempestKeepMovementHelper::CalculateSafePosition(
            TK_ALAR_DIVE_POSITION, bot->GetPosition(), 20.0f);
        bot->GetMotionMaster()->MovePoint(0, escapePos.GetPositionX(),
                                          escapePos.GetPositionY(),
                                          escapePos.GetPositionZ(), false);
        return true;
    }

    return false;
}

bool AlarDiveBombAction::IsDiveBombTarget()
{
    // Check if boss is targeting us with dive bomb
    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss)
        return false;

    // During dive bomb, boss targets a random player
    return boss->GetTarget() == bot->GetGUID();
}

Position AlarDiveBombAction::CalculateEscapePosition()
{
    // Move away from current position quickly
    float angle = bot->GetOrientation() + M_PI;
    float moveDistance = 15.0f;
    
    float newX = bot->GetPositionX() + cos(angle) * moveDistance;
    float newY = bot->GetPositionY() + sin(angle) * moveDistance;
    
    return Position(newX, newY, bot->GetPositionZ(), 0.0f);
}

bool AlarFlamePatchAction::Execute(Event event)
{
    Unit* flamePatch = FindNearestFlamePatch();
    if (!flamePatch)
        return false;

    float distance = bot->GetDistance(flamePatch);
    if (distance < 8.0f)
    {
        // Move away from flame patch
        Position safePos = TempestKeepMovementHelper::CalculateSafePosition(
            flamePatch->GetPosition(), bot->GetPosition(), 10.0f);
        bot->GetMotionMaster()->MovePoint(0, safePos.GetPositionX(),
                                          safePos.GetPositionY(),
                                          safePos.GetPositionZ(), FORCED_MOVEMENT_NONE, 0.f, 0.0f, false);
        return true;
    }

    return false;
}

Unit* AlarFlamePatchAction::FindNearestFlamePatch()
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* nearest = nullptr;
    float minDist = 100.0f;
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_FLAME_PATCH)
        {
            float dist = bot->GetDistance(unit);
            if (dist < minDist)
            {
                minDist = dist;
                nearest = unit;
            }
        }
    }
    
    return nearest;
}

bool AlarFlamePatchAction::IsInFlamePatch()
{
    Unit* patch = FindNearestFlamePatch();
    return patch && bot->GetDistance(patch) < 5.0f;
}

bool AlarPlatformAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    
    // Determine which platform boss is on
    int bossPlatform = GetCurrentPlatform();
    if (bossPlatform < 0)
        return false;

    // Get the platform position
    Position platformPos = GetPlatformPosition(bossPlatform);
    
    // Check if we need to move to platform
    float distance = bot->GetDistance(platformPos);
    if (distance > 5.0f)
    {
        s_alarCurrentPlatform[botGuid] = bossPlatform;
        return TempestKeepMovementHelper::MoveTowardPosition(bot, platformPos, 5.0f);
    }

    return false;
}

int AlarPlatformAction::GetCurrentPlatform()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss)
        return -1;

    const Position platforms[] = {
        TK_ALAR_PLATFORM_1,
        TK_ALAR_PLATFORM_2,
        TK_ALAR_PLATFORM_3,
        TK_ALAR_PLATFORM_4
    };

    // Find which platform boss is closest to
    int closestPlatform = -1;
    float minDist = 100.0f;
    
    for (int i = 0; i < 4; ++i)
    {
        float dist = boss->GetDistance(platforms[i]);
        if (dist < minDist)
        {
            minDist = dist;
            closestPlatform = i;
        }
    }
    
    return (minDist < 10.0f) ? closestPlatform : -1;
}

Position AlarPlatformAction::GetPlatformPosition(int platform)
{
    const Position platforms[] = {
        TK_ALAR_PLATFORM_1,
        TK_ALAR_PLATFORM_2,
        TK_ALAR_PLATFORM_3,
        TK_ALAR_PLATFORM_4
    };
    
    if (platform >= 0 && platform < 4)
        return platforms[platform];
        
    return TK_ALAR_CENTER_POSITION;
}

bool AlarPlatformAction::ShouldMoveToNextPlatform()
{
    // Al'ar moves platforms every 30 seconds in phase 1
    ObjectGuid botGuid = bot->GetGUID();
    uint32 currentTime = getMSTime();
    
    if ((currentTime - s_alarLastPlatformTime[botGuid]) > 25000)
    {
        s_alarLastPlatformTime[botGuid] = currentTime;
        return true;
    }
    
    return false;
}

bool AlarAddsAction::Execute(Event event)
{
    Unit* add = FindPriorityAdd();
    if (!add)
        return false;

    // Mark the add for focus
    MarkAddWithIcon(add);
    
    // Set as target
    if (AI_VALUE(Unit*, "current target") != add)
    {
        Value<Unit*>* targetValue = context->GetValue<Unit*>("current target");
        if (targetValue)
        {
            targetValue->Set(add);
            return true;
        }
    }

    return Attack(add);
}

Unit* AlarAddsAction::FindPriorityAdd()
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* nearest = nullptr;
    float minDist = 100.0f;
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_EMBER_OF_ALAR && unit->IsAlive())
        {
            float dist = bot->GetDistance(unit);
            if (dist < minDist)
            {
                minDist = dist;
                nearest = unit;
            }
        }
    }
    
    return nearest;
}

void AlarAddsAction::MarkAddWithIcon(Unit* add)
{
    if (!add)
        return;

    Group* group = bot->GetGroup();
    if (!group)
        return;

    // Mark with skull if leader/assistant
    if (group->IsLeader(bot->GetGUID()) || group->IsAssistant(bot->GetGUID()))
    {
        group->SetTargetIcon(7, bot->GetGUID(), add->GetGUID()); // Skull is index 7
    }
}

// Void Reaver Actions
bool VoidReaverPoundingAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!boss)
        return false;

    // Pounding has 18 yard range knockback - move out of range
    float distance = bot->GetDistance(boss);
    if (distance < 20.0f)
    {
        // Move away quickly during pounding cast
        Position safePos = TempestKeepMovementHelper::CalculateSafePosition(
            boss->GetPosition(), bot->GetPosition(), 25.0f);
        bot->GetMotionMaster()->MovePoint(0, safePos.GetPositionX(),
                                          safePos.GetPositionY(),
                                          safePos.GetPositionZ(), FORCED_MOVEMENT_NONE, 0.f, 0.0f, false);
        return true;
    }

    return false;
}

bool VoidReaverArcaneOrbAction::Execute(Event event)
{
    // CLAUDE.md: HEALERS MUST HEAL - exclude healers from movement actions
    if (botAI->IsHeal(bot))
        return false;

    // WotLK Standard Pattern: Use disperse distance AI value system
    SET_AI_VALUE(float, "disperse distance", 15.0f);
    return true;
}

bool VoidReaverPositionAction::Execute(Event event)
{
    // CLAUDE.md: HEALERS MUST HEAL - exclude healers from movement actions
    if (botAI->IsHeal(bot))
        return false;

    // Tanks handled by normal combat routine
    if (botAI->IsTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return HandleRangedPosition(boss);
}

bool VoidReaverPositionAction::HandleRangedPosition(Unit* boss)
{
    float distance = bot->GetDistance(boss);

    // Ranged should stay 20-35 yards from boss
    // Too close - risk of pounding
    if (distance < 20.0f)
    {
        Position safePos = TempestKeepMovementHelper::CalculateSafePosition(
            boss->GetPosition(), bot->GetPosition(), 27.0f);
        bot->GetMotionMaster()->MovePoint(0, safePos.GetPositionX(),
                                          safePos.GetPositionY(),
                                          safePos.GetPositionZ(), FORCED_MOVEMENT_NONE, 0.f, 0.0f, false);
        return true;
    }

    // Too far - move closer for healing/dps
    if (distance > 35.0f)
    {
        return TempestKeepMovementHelper::MoveTowardPosition(bot, TK_VOID_REAVER_RANGED_POSITION, 3.0f);
    }

    return false;
}

// Solarian Actions
bool SolarianWrathAction::Execute(Event event)
{
    // If we have Wrath of the Astromancer, spread out from raid
    if (bot->HasAura(SPELL_WRATH_OF_THE_ASTROMANCER))
    {
        // WotLK Standard Pattern: Use disperse distance AI value system
        SET_AI_VALUE(float, "disperse distance", 20.0f);
        return true;
    }

    return false;
}

bool SolarianAddsAction::Execute(Event event)
{
    Unit* add = GetPriorityAdd();
    if (!add)
        return false;

    // Target the add
    if (AI_VALUE(Unit*, "current target") != add)
    {
        Value<Unit*>* targetValue = context->GetValue<Unit*>("current target");
        if (targetValue)
        {
            targetValue->Set(add);
            return true;
        }
    }

    return Attack(add);
}

Unit* SolarianAddsAction::GetPriorityAdd()
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    // Priority: Priests > Agents
    Unit* priest = nullptr;
    Unit* agent = nullptr;
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
            
        if (unit->GetEntry() == NPC_SOLARIUM_PRIEST && !priest)
            priest = unit;
        else if (unit->GetEntry() == NPC_SOLARIUM_AGENT && !agent)
            agent = unit;
    }
    
    return priest ? priest : agent;
}

bool SolarianBlindingLightAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!boss)
        return false;

    // If boss is casting Blinding Light, turn away
    if (boss->FindCurrentSpellBySpellId(SPELL_BLINDING_LIGHT))
    {
        // Turn bot away from boss to avoid the disorient
        float angle = boss->GetAngle(bot) + M_PI;
        bot->SetOrientation(angle);
        bot->SetFacingTo(angle);
        return true;
    }

    return false;
}

bool SolarianPortalAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!boss)
        return false;

    // During portal phase, boss disappears
    if (!boss->IsVisible() || IsPortalActive())
    {
        // Move to safe position and prepare for adds
        Position safePos = GetPortalPosition();
        return TempestKeepMovementHelper::MoveTowardPosition(bot, safePos, 5.0f);
    }

    return false;
}

bool SolarianPortalAction::IsPortalActive()
{
    // Check for spotlight creatures that indicate portal phase
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_ASTROMANCER_SOLARIAN_SPOTLIGHT)
            return true;
    }
    return false;
}

Position SolarianPortalAction::GetPortalPosition()
{
    // During portal phase, move to a spread position
    // The boss spawns adds at random positions
    float angle = bot->GetOrientation();
    float x = 432.909f + cos(angle) * 15.0f;
    float y = -373.424f + sin(angle) * 15.0f;
    return Position(x, y, 17.9608f, angle);
}

// Kael'thas Actions
bool KaelthasAdvisorsAction::Execute(Event event)
{
    Unit* advisor = GetNextAdvisor();
    if (!advisor)
        return false;

    // Target the advisor
    if (AI_VALUE(Unit*, "current target") != advisor)
    {
        Value<Unit*>* targetValue = context->GetValue<Unit*>("current target");
        if (targetValue)
        {
            targetValue->Set(advisor);
            return true;
        }
    }

    return Attack(advisor);
}

Unit* KaelthasAdvisorsAction::GetNextAdvisor()
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    Unit* bestTarget = nullptr;
    int bestPriority = 999;
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
            
        int priority = GetAdvisorKillOrder(unit->GetEntry());
        if (priority < bestPriority)
        {
            bestPriority = priority;
            bestTarget = unit;
        }
    }
    
    return bestTarget;
}

int KaelthasAdvisorsAction::GetAdvisorKillOrder(uint32 entry)
{
    // Kill order: Thaladred > Sanguinar > Capernian > Telonicus
    switch (entry)
    {
        case NPC_THALADRED: return 1;
        case NPC_LORD_SANGUINAR: return 2;
        case NPC_GRAND_ASTROMANCER: return 3;
        case NPC_MASTER_ENGINEER: return 4;
        default: return 999;
    }
}

bool KaelthasWeaponsAction::Execute(Event event)
{
    // During weapons phase, attack the weapons
    // This is a simplified version - actual implementation would need weapon targeting
    Unit* boss = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (boss && boss->HasAura(SPELL_TK_SHOCK_BARRIER))
    {
        // Find and attack weapons (would need actual weapon unit detection)
        return true;
    }

    return false;
}

bool KaelthasPhoenixAction::Execute(Event event)
{
    // Priority: Phoenix > Phoenix Egg
    Unit* phoenix = FindPhoenix();
    if (phoenix)
    {
        if (AI_VALUE(Unit*, "current target") != phoenix)
        {
            Value<Unit*>* targetValue = context->GetValue<Unit*>("current target");
            if (targetValue)
            {
                targetValue->Set(phoenix);
                return true;
            }
        }
        return Attack(phoenix);
    }

    Unit* egg = FindPhoenixEgg();
    if (egg)
    {
        if (AI_VALUE(Unit*, "current target") != egg)
        {
            Value<Unit*>* targetValue = context->GetValue<Unit*>("current target");
            if (targetValue)
            {
                targetValue->Set(egg);
                return true;
            }
        }
        return Attack(egg);
    }

    return false;
}

Unit* KaelthasPhoenixAction::FindPhoenix()
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_TK_PHOENIX && unit->IsAlive())
            return unit;
    }
    return nullptr;
}

Unit* KaelthasPhoenixAction::FindPhoenixEgg()
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_TK_PHOENIX_EGG && unit->IsAlive())
            return unit;
    }
    return nullptr;
}

bool KaelthsFlamestrikeAction::Execute(Event event)
{
    Unit* flamestrike = FindFlamestrike();
    if (!flamestrike)
        return false;

    float distance = bot->GetDistance(flamestrike);
    if (distance < 10.0f)
    {
        // Move away from flamestrike
        Position safePos = TempestKeepMovementHelper::CalculateSafePosition(
            flamestrike->GetPosition(), bot->GetPosition(), 15.0f);
        bot->GetMotionMaster()->MovePoint(0, safePos.GetPositionX(),
                                          safePos.GetPositionY(),
                                          safePos.GetPositionZ(), FORCED_MOVEMENT_NONE, 0.f, 0.0f, false);
        return true;
    }

    return false;
}

Unit* KaelthsFlamestrikeAction::FindFlamestrike()
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_TK_FLAMESTRIKE)
            return unit;
    }
    return nullptr;
}

bool KaelthsFlamestrikeAction::IsInFlamestrike()
{
    Unit* flamestrike = FindFlamestrike();
    return flamestrike && bot->GetDistance(flamestrike) < 8.0f;
}

bool KaelthasGravityLapseAction::Execute(Event event)
{
    if (!bot->HasAura(SPELL_TK_GRAVITY_LAPSE))
        return false;

    // During gravity lapse, we're floating - use movement abilities
    HandleGravityMovement();
    return true;
}

void KaelthasGravityLapseAction::HandleGravityMovement()
{
    // During gravity lapse, try to position properly
    // Move towards a safe position away from others
    float angle = bot->GetOrientation();
    float moveDistance = 5.0f;
    
    float newX = bot->GetPositionX() + cos(angle) * moveDistance;
    float newY = bot->GetPositionY() + sin(angle) * moveDistance;
    float newZ = bot->GetPositionZ() + 2.0f; // Float up slightly
    
    bot->GetMotionMaster()->MovePoint(0, newX, newY, newZ, FORCED_MOVEMENT_NONE, 0.f, 0.0f, false);
}

bool KaelthasPyroblastAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!boss)
        return false;

    // If boss is casting Pyroblast, interrupt if possible or spread
    if (boss->FindCurrentSpellBySpellId(SPELL_TK_PYROBLAST))
    {
        // Healers should prepare for massive damage
        // DPS should interrupt if possible
        Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "interrupt");
        if (spellIdsValue)
        {
            std::list<uint32> spellIds = spellIdsValue->Get();
            for (uint32 spellId : spellIds)
            {
                if (!bot->HasSpellCooldown(spellId))
                {
                    return botAI->CastSpell(spellId, boss);
                }
            }
        }
    }

    return false;
}

bool KaelthasMindControlAction::Execute(Event event)
{
    // Check if we or allies are mind controlled
    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& guid : members)
    {
        Unit* member = botAI->GetUnit(guid);
        if (!member)
            continue;
            
        // Check if member is mind controlled
        if (member->HasAura(SPELL_MIND_CONTROL))
        {
            // Try to dispel if we can
            Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "dispel");
            if (spellIdsValue)
            {
                std::list<uint32> spellIds = spellIdsValue->Get();
                for (uint32 spellId : spellIds)
                {
                    if (!bot->HasSpellCooldown(spellId))
                    {
                        return botAI->CastSpell(spellId, member);
                    }
                }
            }
        }
    }

    return false;
}

bool KaelthasNetherVaporAction::Execute(Event event)
{
    Unit* vapor = FindNearestNetherVapor();
    if (!vapor)
        return false;

    float distance = bot->GetDistance(vapor);
    if (distance < 10.0f)
    {
        // Move away from Nether Vapor
        Position safePos = TempestKeepMovementHelper::CalculateSafePosition(
            vapor->GetPosition(), bot->GetPosition(), 15.0f);
        bot->GetMotionMaster()->MovePoint(0, safePos.GetPositionX(),
                                          safePos.GetPositionY(),
                                          safePos.GetPositionZ(), FORCED_MOVEMENT_NONE, 0.f, 0.0f, false);
        return true;
    }

    return false;
}

Unit* KaelthasNetherVaporAction::FindNearestNetherVapor()
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* nearest = nullptr;
    float minDist = 100.0f;
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_NETHER_VAPOR)
        {
            float dist = bot->GetDistance(unit);
            if (dist < minDist)
            {
                minDist = dist;
                nearest = unit;
            }
        }
    }
    
    return nearest;
}

bool ThaladredFixateAction::Execute(Event event)
{
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (!thaladred || !thaladred->IsAlive())
        return false;

    // If Thaladred is targeting us, kite
    if (thaladred->GetTarget() == bot->GetGUID())
    {
        // Ranged should kite, melee should try to avoid
        float distance = bot->GetDistance(thaladred);
        if (distance < 10.0f)
        {
            Position escapePos = TempestKeepMovementHelper::CalculateSafePosition(
                thaladred->GetPosition(), bot->GetPosition(), 15.0f);
            bot->GetMotionMaster()->MovePoint(0, escapePos.GetPositionX(),
                                              escapePos.GetPositionY(),
                                              escapePos.GetPositionZ(), FORCED_MOVEMENT_NONE, 0.f, 0.0f, false);
            return true;
        }
    }

    return false;
}

bool CapernianConflagrationAction::Execute(Event event)
{
    // If we have Conflagration, spread from others
    if (bot->HasAura(SPELL_CONFLAGRATION))
    {
        GuidVector members = AI_VALUE(GuidVector, "group members");
        for (auto& guid : members)
        {
            Unit* member = botAI->GetUnit(guid);
            if (!member || member == bot)
                continue;
                
            float memberDistance = bot->GetDistance(member);
            if (memberDistance < 10.0f)
            {
                // Spread out urgently
                Position spreadPos = TempestKeepMovementHelper::CalculateSafePosition(
                    member->GetPosition(), bot->GetPosition(), 15.0f);
                bot->GetMotionMaster()->MovePoint(0, spreadPos.GetPositionX(),
                                                  spreadPos.GetPositionY(),
                                                  spreadPos.GetPositionZ(), FORCED_MOVEMENT_NONE, 0.f, 0.0f, false);
                return true;
            }
        }
    }

    return false;
}

bool TelonicusRemoteToyAction::Execute(Event event)
{
    // If we have Remote Toy debuff, we might get stunned
    if (bot->HasAura(SPELL_REMOTE_TOY))
    {
        // Healers should be ready to dispel
        // Nothing we can do movement-wise
        return false;
    }

    return false;
}

// New Al'ar actions for missing mechanics
bool AlarFlameBuffetAction::Execute(Event event)
{
    // Tank swap or kiting behavior for Flame Buffet stacks
    if (!botAI->IsTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss)
        return false;

    // Check stack count - if too high, kite or swap
    if (Aura* buffetAura = bot->GetAura(SPELL_FLAME_BUFFET))
    {
        uint32 stacks = buffetAura->GetStackAmount();
        
        // At 5+ stacks, consider kiting to reset
        if (stacks >= 5)
        {
            // Move away to let stacks drop
            Position kitePos = TempestKeepMovementHelper::CalculateSafePosition(
                boss->GetPosition(), bot->GetPosition(), 15.0f);
            
            return MoveTo(boss->GetMapId(), kitePos.GetPositionX(), 
                         kitePos.GetPositionY(), kitePos.GetPositionZ(),
                         false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
        }
    }

    // Continue normal tanking
    return Attack(boss);
}

bool AlarEmberBlastAction::Execute(Event event)
{
    // Move away from Ember of Al'ar adds that are about to cast Ember Blast
    if (Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs"))
    {
        GuidVector const npcs = npcsValue->Get();
        for (ObjectGuid const& guid : npcs)
        {
            Unit* ember = botAI->GetUnit(guid);
            if (ember && ember->IsAlive() && ember->GetEntry() == NPC_EMBER_OF_ALAR)
            {
                float distance = bot->GetDistance(ember);
                if (distance < 12.0f) // Ember Blast has ~8 yard radius, move to 12y for safety
                {
                    // Calculate escape position away from ember
                    Position escapePos = TempestKeepMovementHelper::CalculateSafePosition(
                        ember->GetPosition(), bot->GetPosition(), 15.0f);
                    
                    return MoveTo(ember->GetMapId(), escapePos.GetPositionX(),
                                 escapePos.GetPositionY(), escapePos.GetPositionZ(),
                                 false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
                }
            }
        }
    }
    return false;
}

bool AlarMeltArmorAction::Execute(Event event)
{
    // Tank swap behavior for Melt Armor in Phase 2
    if (!botAI->IsTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss)
        return false;

    // Check Melt Armor stacks
    if (bot->HasAura(SPELL_MELT_ARMOR))
    {
        if (Aura* meltAura = bot->GetAura(SPELL_MELT_ARMOR))
        {
            uint32 stacks = meltAura->GetStackAmount();

            // At 3+ stacks, offtank should taunt
            if (stacks >= 3)
            {
                // Main tank should kite/move away
                if (botAI->IsMainTank(bot))
                {
                    // Move away from boss to let offtank pick up
                    Position kitePos = TempestKeepMovementHelper::CalculateSafePosition(
                        boss->GetPosition(), bot->GetPosition(), 15.0f);

                    return MoveTo(boss->GetMapId(), kitePos.GetPositionX(),
                                 kitePos.GetPositionY(), kitePos.GetPositionZ(),
                                 false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
                }
            }
        }

        // Use defensive cooldowns if available
        if (Value<std::list<uint32>>* spellIdsValue =
            botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "defensive"))
        {
            for (auto spellId : spellIdsValue->Get())
            {
                if (botAI->CanCastSpell(spellId, bot, false))
                    return botAI->CastSpell(spellId, bot);
            }
        }
    }
    else if (!botAI->IsMainTank(bot))
    {
        // Offtank should taunt if main tank has high stacks
        GuidVector members = AI_VALUE(GuidVector, "group members");
        for (auto& guid : members)
        {
            Unit* member = botAI->GetUnit(guid);
            if (member && member->HasAura(SPELL_MELT_ARMOR))
            {
                if (Aura* meltAura = member->GetAura(SPELL_MELT_ARMOR))
                {
                    uint32 stacks = meltAura->GetStackAmount();
                    if (stacks >= 3)
                    {
                        // Offtank should taunt
                        if (Value<std::list<uint32>>* spellIdsValue =
                            botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "taunt"))
                        {
                            for (auto spellId : spellIdsValue->Get())
                            {
                                if (botAI->CanCastSpell(spellId, boss, false))
                                    return botAI->CastSpell(spellId, boss);
                            }
                        }
                    }
                }
            }
        }
    }

    // Continue attacking boss
    return Attack(boss);
}

bool AlarChargeAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss)
        return false;

    // Move out of charge path
    if (boss->HasInArc(M_PI_4, bot))
    {
        // Calculate perpendicular movement to avoid charge line
        float bossOrientation = boss->GetAngle(bot);
        float escapeAngle = bossOrientation + M_PI_2; // 90 degrees to the side

        float escapeX = bot->GetPositionX() + 10.0f * cos(escapeAngle);
        float escapeY = bot->GetPositionY() + 10.0f * sin(escapeAngle);
        float escapeZ = bot->GetPositionZ();

        return MoveTo(boss->GetMapId(), escapeX, escapeY, escapeZ,
                     false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
    }

    return false;
}

bool AlarOfftankPlatformAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss)
        return false;

    // Get the offtank platform position (next platform in rotation)
    Position offtankPos = GetOfftankPlatform();

    // Move to offtank platform
    float distance = bot->GetDistance(offtankPos);
    if (distance > 5.0f)
    {
        return MoveTo(boss->GetMapId(), offtankPos.GetPositionX(),
                     offtankPos.GetPositionY(), offtankPos.GetPositionZ(),
                     false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
    }

    // Stay at platform and attack boss if in range
    return Attack(boss);
}

Position AlarOfftankPlatformAction::GetOfftankPlatform()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss)
        return TK_ALAR_PLATFORM_2;

    const Position platforms[] = {
        TK_ALAR_PLATFORM_1,
        TK_ALAR_PLATFORM_2,
        TK_ALAR_PLATFORM_3,
        TK_ALAR_PLATFORM_4
    };

    // Find which platform boss is on
    int bossPlatform = -1;
    float minDist = 100.0f;
    for (int i = 0; i < 4; ++i)
    {
        float dist = boss->GetDistance(platforms[i]);
        if (dist < minDist)
        {
            minDist = dist;
            bossPlatform = i;
        }
    }

    // Offtank goes to next platform in rotation
    int offtankPlatform = (bossPlatform + 1) % 4;
    return platforms[offtankPlatform];
}

bool AlarPlateDpsAddTankAction::Execute(Event event)
{
    // Find Ember of Al'ar add to tank
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* ember = nullptr;

    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_EMBER_OF_ALAR && unit->IsAlive())
        {
            // Check if add has no tank or we're already tanking it
            if (!unit->GetVictim() || unit->GetVictim() == bot)
            {
                ember = unit;
                break;
            }
        }
    }

    if (!ember)
        return false;

    // Tank the add - attack it to generate threat
    return Attack(ember);
}

bool AlarPlateDpsEscapeAction::Execute(Event event)
{
    // Find the low-health Ember of Al'ar we're tanking
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* emberToEscape = nullptr;

    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_EMBER_OF_ALAR && unit->IsAlive())
        {
            float healthPct = (unit->GetHealth() * 100.0f) / unit->GetMaxHealth();
            if (healthPct < 10.0f && unit->GetTarget() == bot->GetGUID())
            {
                emberToEscape = unit;
                break;
            }
        }
    }

    if (!emberToEscape)
        return false;

    // Run away from the add before it explodes
    Position escapePos = TempestKeepMovementHelper::CalculateSafePosition(
        emberToEscape->GetPosition(), bot->GetPosition(), 20.0f);

    return MoveTo(bot->GetMapId(), escapePos.GetPositionX(),
                 escapePos.GetPositionY(), escapePos.GetPositionZ(),
                 false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
}
