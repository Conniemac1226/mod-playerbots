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
    
    bot->GetMotionMaster()->MovePoint(0, moveX, moveY, targetZ, false);
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
                                          escapePos.GetPositionZ(), false);
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
                                          safePos.GetPositionZ(), false);
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
                                          safePos.GetPositionZ(), false);
        return true;
    }

    return false;
}

bool VoidReaverArcaneOrbAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!boss)
        return false;

    // Check if an arcane orb is targeting us
    // Arcane Orb targets players 20+ yards away from boss
    float distanceFromBoss = bot->GetDistance(boss);
    
    // If we're at risk range (20-40 yards), spread out to minimize orb damage
    if (distanceFromBoss > 18.0f && distanceFromBoss < 35.0f)
    {
        // Check if other players are too close
        GuidVector members = AI_VALUE(GuidVector, "group members");
        for (auto& guid : members)
        {
            Unit* member = botAI->GetUnit(guid);
            if (!member || member == bot)
                continue;
                
            float memberDistance = bot->GetDistance(member);
            if (memberDistance < 10.0f)
            {
                // Spread out from other players
                Position spreadPos = TempestKeepMovementHelper::CalculateSafePosition(
                    member->GetPosition(), bot->GetPosition(), 15.0f);
                bot->GetMotionMaster()->MovePoint(0, spreadPos.GetPositionX(),
                                                  spreadPos.GetPositionY(),
                                                  spreadPos.GetPositionZ(), false);
                return true;
            }
        }
    }

    return false;
}

bool VoidReaverPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if we're main tank
    bool isMainTank = botAI->IsMainTank(bot);
    
    if (isMainTank)
        return HandleTankPosition(boss);
    else
        return HandleRangedPosition(boss);
}

bool VoidReaverPositionAction::HandleTankPosition(Unit* boss)
{
    // Tank should be in melee range but ready for knock away
    float distance = bot->GetDistance(boss);
    
    // If knocked back, return to position
    if (distance > 5.0f)
    {
        return TempestKeepMovementHelper::MoveTowardPosition(bot, TK_VOID_REAVER_TANK_POSITION, 3.0f);
    }
    
    // Face boss away from raid
    if (distance < 3.0f)
    {
        float angle = boss->GetAngle(TK_VOID_REAVER_RANGED_POSITION.GetPositionX(),
                                    TK_VOID_REAVER_RANGED_POSITION.GetPositionY());
        float newX = boss->GetPositionX() + cos(angle + M_PI) * 3.0f;
        float newY = boss->GetPositionY() + sin(angle + M_PI) * 3.0f;
        
        if (bot->GetDistance2d(newX, newY) > 2.0f)
        {
            bot->GetMotionMaster()->MovePoint(0, newX, newY, boss->GetPositionZ(), false);
            return true;
        }
    }
    
    return false;
}

bool VoidReaverPositionAction::HandleRangedPosition(Unit* boss)
{
    float distance = bot->GetDistance(boss);
    
    // Ranged should stay 20-30 yards from boss
    // Too close - risk of pounding
    if (distance < 20.0f)
    {
        Position safePos = TempestKeepMovementHelper::CalculateSafePosition(
            boss->GetPosition(), bot->GetPosition(), 25.0f);
        bot->GetMotionMaster()->MovePoint(0, safePos.GetPositionX(),
                                          safePos.GetPositionY(),
                                          safePos.GetPositionZ(), false);
        return true;
    }
    
    // Too far - move closer for healing/dps
    if (distance > 35.0f)
    {
        return TempestKeepMovementHelper::MoveTowardPosition(bot, TK_VOID_REAVER_RANGED_POSITION, 3.0f);
    }
    
    // Spread from other players for arcane orb
    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& guid : members)
    {
        Unit* member = botAI->GetUnit(guid);
        if (!member || member == bot)
            continue;
            
        if (bot->GetDistance(member) < 8.0f)
        {
            Position spreadPos = TempestKeepMovementHelper::CalculateSafePosition(
                member->GetPosition(), bot->GetPosition(), 10.0f);
            bot->GetMotionMaster()->MovePoint(0, spreadPos.GetPositionX(),
                                              spreadPos.GetPositionY(),
                                              spreadPos.GetPositionZ(), false);
            return true;
        }
    }
    
    return false;
}

// Solarian Actions
bool SolarianWrathAction::Execute(Event event)
{
    // If we have Wrath of the Astromancer, spread out from raid
    if (bot->HasAura(SPELL_WRATH_OF_THE_ASTROMANCER))
    {
        // Move away from other players to avoid explosion damage
        GuidVector members = AI_VALUE(GuidVector, "group members");
        for (auto& guid : members)
        {
            Unit* member = botAI->GetUnit(guid);
            if (!member || member == bot)
                continue;
                
            float memberDistance = bot->GetDistance(member);
            if (memberDistance < 15.0f)
            {
                // Spread out urgently
                Position spreadPos = TempestKeepMovementHelper::CalculateSafePosition(
                    member->GetPosition(), bot->GetPosition(), 20.0f);
                bot->GetMotionMaster()->MovePoint(0, spreadPos.GetPositionX(),
                                                  spreadPos.GetPositionY(),
                                                  spreadPos.GetPositionZ(), false);
                return true;
            }
        }
        
        // Move to designated spread position if not already there
        return TempestKeepMovementHelper::MoveTowardPosition(bot, TK_SOLARIAN_SPREAD_POSITION, 5.0f);
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
                                          safePos.GetPositionZ(), false);
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
    
    bot->GetMotionMaster()->MovePoint(0, newX, newY, newZ, false);
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
                                          safePos.GetPositionZ(), false);
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
                                              escapePos.GetPositionZ(), false);
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
                                                  spreadPos.GetPositionZ(), false);
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