#include "SerpentshrineActions.h"
#include "SerpentshrineTriggers.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "ScriptedCreature.h"
#include "AiObjectContext.h"
#include "Value.h"

std::map<ObjectGuid, uint32> g_hydross_lastMoveTime;
std::map<ObjectGuid, uint8> g_hydross_markStacks;
std::map<ObjectGuid, bool> g_hydross_transitionNeeded;

bool HydrossAvoidMarkOfHydrossAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    uint8 stacks = 0;
    if (bot->HasAura(SPELL_MARK_OF_HYDROSS1)) stacks = 1;
    else if (bot->HasAura(SPELL_MARK_OF_HYDROSS2)) stacks = 2;
    else if (bot->HasAura(SPELL_MARK_OF_HYDROSS3)) stacks = 3;
    else if (bot->HasAura(SPELL_MARK_OF_HYDROSS4)) stacks = 4;
    else if (bot->HasAura(SPELL_MARK_OF_HYDROSS5)) stacks = 5;
    else if (bot->HasAura(SPELL_MARK_OF_HYDROSS6)) stacks = 6;

    ObjectGuid botGuid = bot->GetGUID();
    if (botGuid.IsEmpty())
    {
        return false;
    }
    
    g_hydross_markStacks[botGuid] = stacks;

    if (botAI->IsTank(bot) && stacks >= 4)
    {
        g_hydross_transitionNeeded[bot->GetGUID()] = true;
        return false;
    }

    if (!botAI->IsTank(bot) && stacks >= 3)
    {
        float distance = bot->GetDistance(boss);
        if (distance < 20.0f)
        {
            Position movePos = bot->GetPosition();
            float angle = bot->GetAngle(boss) + M_PI;
            movePos.m_positionX += cos(angle) * 10.0f;
            movePos.m_positionY += sin(angle) * 10.0f;
            
            return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                         false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool HydrossAvoidMarkOfCorruptionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    uint8 stacks = 0;
    if (bot->HasAura(SPELL_MARK_OF_CORRUPTION1)) stacks = 1;
    else if (bot->HasAura(SPELL_MARK_OF_CORRUPTION2)) stacks = 2;
    else if (bot->HasAura(SPELL_MARK_OF_CORRUPTION3)) stacks = 3;
    else if (bot->HasAura(SPELL_MARK_OF_CORRUPTION4)) stacks = 4;
    else if (bot->HasAura(SPELL_MARK_OF_CORRUPTION5)) stacks = 5;
    else if (bot->HasAura(SPELL_MARK_OF_CORRUPTION6)) stacks = 6;

    g_hydross_markStacks[bot->GetGUID()] = stacks;

    if (botAI->IsTank(bot) && stacks >= 4)
    {
        g_hydross_transitionNeeded[bot->GetGUID()] = true;
        return false;
    }

    if (!botAI->IsTank(bot) && stacks >= 3)
    {
        float distance = bot->GetDistance(boss);
        if (distance < 20.0f)
        {
            Position movePos = bot->GetPosition();
            float angle = bot->GetAngle(boss) + M_PI;
            movePos.m_positionX += cos(angle) * 10.0f;
            movePos.m_positionY += sin(angle) * 10.0f;
            
            return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                         false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool HydrossWaterTombSpreadAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // React immediately to cast or pre-cast
    if (boss->FindCurrentSpellBySpellId(SPELL_WATER_TOMB) || 
        (!boss->HasAura(SPELL_HYDROSS_CORRUPTION) && boss->HasUnitState(UNIT_STATE_CASTING)))
    {
        Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
        if (!membersValue)
        {
            return false;
        }
        GuidVector members = membersValue->Get();
        Unit* closestAlly = nullptr;
        float minDistance = 100.0f;
        
        for (ObjectGuid const& member : members)
        {
            Unit* player = botAI->GetUnit(member);
            if (player && player != bot && player->IsAlive())
            {
                float distance = bot->GetDistance(player);
                if (distance < minDistance)
                {
                    closestAlly = player;
                    minDistance = distance;
                }
            }
        }
        
        if (closestAlly && minDistance < 8.0f)
        {
            Position movePos = bot->GetPosition();
            float angle = bot->GetAngle(closestAlly) + M_PI;
            movePos.m_positionX += cos(angle) * 12.0f; // Move further for safety
            movePos.m_positionY += sin(angle) * 12.0f;
            
            return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                         false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool HydrossVileSludgeSpreadAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // React to cast or pre-cast in poison phase
    if (boss->HasAura(SPELL_HYDROSS_CORRUPTION))
    {
        float distance = bot->GetDistance(boss);
        bool shouldMove = false;
        float moveDistance = 10.0f;
        
        // Immediate reaction to Vile Sludge cast
        if (boss->FindCurrentSpellBySpellId(SPELL_VILE_SLUDGE))
        {
            shouldMove = distance < 20.0f;
            moveDistance = 15.0f; // Move further when actually casting
        }
        // Pre-emptive movement when boss is casting in poison form
        else if (boss->HasUnitState(UNIT_STATE_CASTING))
        {
            shouldMove = distance < 12.0f;
        }
        
        if (shouldMove)
        {
            Position movePos = bot->GetPosition();
            float angle = bot->GetAngle(boss) + M_PI;
            movePos.m_positionX += cos(angle) * moveDistance;
            movePos.m_positionY += sin(angle) * moveDistance;
            
            return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                         false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool HydrossKillAddsAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
    {
        return false;
    }
    GuidVector npcs = npcsValue->Get();
    
    Unit* pureSpawn = nullptr;
    Unit* taintedSpawn = nullptr;
    float minPureDistance = 100.0f;
    float minTaintedDistance = 100.0f;

    for (ObjectGuid const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || !unit->IsAlive())
            continue;

        float distance = bot->GetDistance(unit);
        
        if (unit->GetEntry() == NPC_PURE_SPAWN_OF_HYDROSS && distance < minPureDistance)
        {
            pureSpawn = unit;
            minPureDistance = distance;
        }
        else if (unit->GetEntry() == NPC_TAINTED_SPAWN_OF_HYDROSS && distance < minTaintedDistance)
        {
            taintedSpawn = unit;
            minTaintedDistance = distance;
        }
    }

    Unit* target = nullptr;
    if (pureSpawn && taintedSpawn)
    {
        target = (minPureDistance < minTaintedDistance) ? pureSpawn : taintedSpawn;
    }
    else if (pureSpawn)
    {
        target = pureSpawn;
    }
    else if (taintedSpawn)
    {
        target = taintedSpawn;
    }

    if (target)
    {
        if (AI_VALUE(Unit*, "current target") != target)
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(target);
        }
        return Attack(target);
    }

    return false;
}

bool HydrossPositionTankAction::Execute(Event event)
{
    if (!botAI->IsTank(bot))
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    bool hasCorruption = boss->HasAura(SPELL_HYDROSS_CORRUPTION);
    bool hasBlueBeam = boss->HasAura(SPELL_BLUE_BEAM);
    
    float desiredX, desiredY, desiredZ;
    
    if (hasCorruption)
    {
        desiredX = -29.0f;
        desiredY = -923.0f;
        desiredZ = 42.0f;
    }
    else
    {
        desiredX = 71.0f;
        desiredY = -883.0f;
        desiredZ = 41.0f;
    }
    
    Position desiredPos(desiredX, desiredY, desiredZ);
    float distance = bot->GetDistance(desiredPos);
    
    if (distance > 5.0f)
    {
        return MoveTo(bot->GetMapId(), desiredX, desiredY, desiredZ,
                     false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }
    
    return false;
}

bool HydrossTransitionControlAction::Execute(Event event)
{
    if (!botAI->IsTank(bot))
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    ObjectGuid botGuid = bot->GetGUID();
    if (!g_hydross_transitionNeeded[botGuid])
    {
        return false;
    }

    bool hasCorruption = boss->HasAura(SPELL_HYDROSS_CORRUPTION);
    
    float targetX, targetY, targetZ;
    
    if (hasCorruption)
    {
        targetX = 71.0f;
        targetY = -883.0f;
        targetZ = 41.0f;
    }
    else
    {
        targetX = -29.0f;
        targetY = -923.0f;
        targetZ = 42.0f;
    }
    
    uint32 currentTime = getMSTime();
    if (currentTime - g_hydross_lastMoveTime[botGuid] > 10000)
    {
        g_hydross_transitionNeeded[botGuid] = false;
        g_hydross_markStacks[botGuid] = 0;
    }
    
    g_hydross_lastMoveTime[botGuid] = currentTime;
    
    return MoveTo(bot->GetMapId(), targetX, targetY, targetZ,
                 false, false, false, true, MovementPriority::MOVEMENT_FORCED);
}

// The Lurker Below Actions
std::map<ObjectGuid, uint32> g_lurker_lastSpoutTime;
std::map<ObjectGuid, bool> g_lurker_inWater;

bool LurkerSpoutAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Check if boss is casting or has spout visual
    if (boss->HasAura(SPELL_LURKER_SPOUT_VISUAL) || 
        boss->FindCurrentSpellBySpellId(SPELL_LURKER_SPOUT_PERIODIC_1) ||
        boss->FindCurrentSpellBySpellId(SPELL_LURKER_SPOUT_PERIODIC_2))
    {
        ObjectGuid botGuid = bot->GetGUID();
        uint32 currentTime = getMSTime();
        
        // Jump into water to avoid spout if not already in water
        if (!bot->IsInWater())
        {
            g_lurker_lastSpoutTime[botGuid] = currentTime;
            
            // Find nearest water position (jump off platform)
            Position jumpPos = bot->GetPosition();
            float angle = bot->GetAngle(boss);
            
            // Jump away from boss into water
            jumpPos.m_positionX += cos(angle + M_PI) * 15.0f;
            jumpPos.m_positionY += sin(angle + M_PI) * 15.0f;
            jumpPos.m_positionZ -= 5.0f; // Jump down into water
            
            return MoveTo(bot->GetMapId(), jumpPos.m_positionX, jumpPos.m_positionY, jumpPos.m_positionZ,
                         false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
        g_lurker_inWater[botGuid] = true;
    }
    else if (g_lurker_inWater[bot->GetGUID()])
    {
        // Spout ended, get back on platform
        g_lurker_inWater[bot->GetGUID()] = false;
        
        // Move back to platform
        float platformZ = -19.18f; // Platform height from boss script
        Position returnPos = bot->GetPosition();
        returnPos.m_positionZ = platformZ;
        
        return MoveTo(bot->GetMapId(), returnPos.m_positionX, returnPos.m_positionY, returnPos.m_positionZ,
                     false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool LurkerWhirlAvoidAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Only melee needs to avoid whirl
    if (!botAI->IsMelee(bot))
    {
        return false;
    }

    // Check if boss is casting whirl
    if (boss->FindCurrentSpellBySpellId(SPELL_LURKER_WHIRL) || boss->HasAura(SPELL_LURKER_WHIRL))
    {
        float distance = bot->GetDistance(boss);
        if (distance < 8.0f) // Whirl has 5 yard range + buffer
        {
            Position movePos = bot->GetPosition();
            float angle = bot->GetAngle(boss) + M_PI;
            movePos.m_positionX += cos(angle) * 10.0f;
            movePos.m_positionY += sin(angle) * 10.0f;
            
            return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                         false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool LurkerGeyserSpreadAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Check if boss is casting geyser
    if (boss->FindCurrentSpellBySpellId(SPELL_LURKER_GEYSER))
    {
        // Spread out from other players to minimize geyser damage
        Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
        if (!membersValue)
        {
            return false;
        }
        GuidVector members = membersValue->Get();
        
        for (ObjectGuid const& member : members)
        {
            Unit* player = botAI->GetUnit(member);
            if (player && player != bot && player->IsAlive())
            {
                float distance = bot->GetDistance(player);
                if (distance < 10.0f) // Geyser has knockback, maintain distance
                {
                    Position movePos = bot->GetPosition();
                    float angle = bot->GetAngle(player) + M_PI;
                    movePos.m_positionX += cos(angle) * 8.0f;
                    movePos.m_positionY += sin(angle) * 8.0f;
                    
                    return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                                 false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
                }
            }
        }
    }

    return false;
}

bool LurkerKillAddsAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Only attack adds when boss is submerged
    if (!boss->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
    {
        return false;
    }

    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
    {
        return false;
    }
    GuidVector npcs = npcsValue->Get();
    
    Unit* guardian = nullptr;
    Unit* ambusher = nullptr;
    float minGuardianDistance = 100.0f;
    float minAmbusherDistance = 100.0f;

    for (ObjectGuid const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || !unit->IsAlive())
            continue;

        float distance = bot->GetDistance(unit);
        
        // Guardians are priority (harder hitting)
        if (unit->GetEntry() == NPC_COILFANG_GUARDIAN && distance < minGuardianDistance)
        {
            guardian = unit;
            minGuardianDistance = distance;
        }
        else if (unit->GetEntry() == NPC_COILFANG_AMBUSHER && distance < minAmbusherDistance)
        {
            ambusher = unit;
            minAmbusherDistance = distance;
        }
    }

    Unit* target = guardian ? guardian : ambusher;

    if (target)
    {
        if (AI_VALUE(Unit*, "current target") != target)
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(target);
        }
        return Attack(target);
    }

    return false;
}

bool LurkerPositionAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Don't reposition during spout or when submerged
    if (boss->HasAura(SPELL_LURKER_SPOUT_VISUAL) || boss->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
    {
        return false;
    }

    float desiredDistance = botAI->IsMelee(bot) ? 5.0f : 25.0f;
    float currentDistance = bot->GetDistance(boss);
    
    // Position properly based on role
    if (fabs(currentDistance - desiredDistance) > 3.0f)
    {
        Position movePos = boss->GetPosition();
        float angle = boss->GetAngle(bot);
        movePos.m_positionX += cos(angle) * desiredDistance;
        movePos.m_positionY += sin(angle) * desiredDistance;
        movePos.m_positionZ = -19.18f; // Platform height
        
        return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                     false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

// Leotheras the Blind Actions
std::map<ObjectGuid, uint32> g_leotheras_lastWhirlwindTime;
std::map<ObjectGuid, bool> g_leotheras_hasDemon;

bool LeotherasWhirlwindAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Check if boss is casting or has whirlwind
    if (boss->FindCurrentSpellBySpellId(SPELL_LEOTHERAS_WHIRLWIND) || boss->HasAura(SPELL_LEOTHERAS_WHIRLWIND))
    {
        ObjectGuid botGuid = bot->GetGUID();
        uint32 currentTime = getMSTime();
        
        // Move away from whirlwind
        float distance = bot->GetDistance(boss);
        if (distance < 15.0f) // Whirlwind range + safety margin
        {
            g_leotheras_lastWhirlwindTime[botGuid] = currentTime;
            
            Position movePos = bot->GetPosition();
            float angle = bot->GetAngle(boss) + M_PI;
            movePos.m_positionX += cos(angle) * 20.0f;
            movePos.m_positionY += sin(angle) * 20.0f;
            
            return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                         false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool LeotherasChaosBlastAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Check if boss is in demon form and casting chaos blast
    if (boss->HasAura(SPELL_LEOTHERAS_METAMORPHOSIS) && 
        (boss->FindCurrentSpellBySpellId(SPELL_LEOTHERAS_CHAOS_BLAST) || boss->HasUnitState(UNIT_STATE_CASTING)))
    {
        // Spread out to minimize chain damage
        Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
        if (!membersValue)
        {
            return false;
        }
        GuidVector members = membersValue->Get();
        
        for (ObjectGuid const& member : members)
        {
            Unit* player = botAI->GetUnit(member);
            if (player && player != bot && player->IsAlive())
            {
                float distance = bot->GetDistance(player);
                if (distance < 8.0f) // Chaos blast chain range
                {
                    Position movePos = bot->GetPosition();
                    float angle = bot->GetAngle(player) + M_PI;
                    movePos.m_positionX += cos(angle) * 10.0f;
                    movePos.m_positionY += sin(angle) * 10.0f;
                    
                    return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                                 false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
                }
            }
        }
    }

    return false;
}

bool LeotherasInnerDemonAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    // Look for Inner Demon that belongs to this bot
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
    {
        return false;
    }
    GuidVector npcs = npcsValue->Get();
    
    Unit* myDemon = nullptr;
    float minDistance = 100.0f;

    for (ObjectGuid const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_INNER_DEMON)
        {
            // Inner Demons can only be damaged by their summoned player
            // Check if this demon is summoned for this bot
            if (unit->ToCreature() && unit->ToCreature()->GetSummonerGUID() == bot->GetGUID())
            {
                float distance = bot->GetDistance(unit);
                if (distance < minDistance)
                {
                    myDemon = unit;
                    minDistance = distance;
                    g_leotheras_hasDemon[bot->GetGUID()] = true;
                }
            }
        }
    }

    if (myDemon)
    {
        // Priority target - must kill inner demon quickly
        if (AI_VALUE(Unit*, "current target") != myDemon)
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(myDemon);
        }
        return Attack(myDemon);
    }
    else if (g_leotheras_hasDemon[bot->GetGUID()])
    {
        // Demon was killed, clear flag
        g_leotheras_hasDemon[bot->GetGUID()] = false;
    }

    return false;
}

bool LeotherasShadowAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // At 15% health, shadow spawns
    if (boss->GetHealthPct() <= 15.0f)
    {
        Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
        if (!npcsValue)
        {
            return false;
        }
        GuidVector npcs = npcsValue->Get();
        
        Unit* shadow = nullptr;
        float minDistance = 100.0f;

        for (ObjectGuid const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->GetEntry() == NPC_SHADOW_OF_LEOTHERAS)
            {
                float distance = bot->GetDistance(unit);
                if (distance < minDistance)
                {
                    shadow = unit;
                    minDistance = distance;
                }
            }
        }

        // Ranged should focus shadow, melee on boss
        if (shadow && !botAI->IsMelee(bot))
        {
            if (AI_VALUE(Unit*, "current target") != shadow)
            {
                botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(shadow);
            }
            return Attack(shadow);
        }
    }

    return false;
}

bool LeotherasPositionAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Don't reposition during whirlwind
    if (boss->HasAura(SPELL_LEOTHERAS_WHIRLWIND))
    {
        return false;
    }

    // Different positioning for demon form
    float desiredDistance;
    if (boss->HasAura(SPELL_LEOTHERAS_METAMORPHOSIS))
    {
        // Demon form - stay at range for chaos blast
        desiredDistance = botAI->IsRanged(bot) ? 30.0f : 35.0f;
    }
    else
    {
        // Elf form - normal positioning
        desiredDistance = botAI->IsMelee(bot) ? 5.0f : 25.0f;
    }

    float currentDistance = bot->GetDistance(boss);
    
    if (fabs(currentDistance - desiredDistance) > 3.0f)
    {
        Position movePos = boss->GetPosition();
        float angle = boss->GetAngle(bot);
        movePos.m_positionX += cos(angle) * desiredDistance;
        movePos.m_positionY += sin(angle) * desiredDistance;
        
        return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                     false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

// Fathom-Lord Karathress Actions
bool KarathressCataclysmicBoltAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Cataclysmic Bolt targets mana users - move away to reduce damage
    if (bot->getPowerType() == POWER_MANA && boss->FindCurrentSpellBySpellId(SPELL_KARATHRESS_CATACLYSMIC_BOLT))
    {
        float distance = bot->GetDistance(boss);
        if (distance < 30.0f) // Try to outrange if possible
        {
            Position movePos = bot->GetPosition();
            float angle = bot->GetAngle(boss) + M_PI;
            movePos.m_positionX += cos(angle) * 10.0f;
            movePos.m_positionY += sin(angle) * 10.0f;
            
            return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                         false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool KarathressSearNovaAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Sear Nova is a melee AoE - spread out
    if (boss->FindCurrentSpellBySpellId(SPELL_KARATHRESS_SEAR_NOVA) || 
        (boss->HasUnitState(UNIT_STATE_CASTING) && bot->GetDistance(boss) < 10.0f))
    {
        float distance = bot->GetDistance(boss);
        if (distance < 10.0f)
        {
            Position movePos = bot->GetPosition();
            float angle = bot->GetAngle(boss) + M_PI;
            movePos.m_positionX += cos(angle) * 15.0f;
            movePos.m_positionY += sin(angle) * 15.0f;
            
            return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                         false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool KarathressAdvisorsAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
    {
        return false;
    }
    GuidVector npcs = npcsValue->Get();
    
    // Kill order: Tidalvess (shaman) -> Sharkkis (hunter) -> Caribdis (priest) -> Karathress
    Unit* tidalvess = nullptr;
    Unit* sharkkis = nullptr;
    Unit* caribdis = nullptr;
    
    float minTidalvessDistance = 100.0f;
    float minSharkkisDistance = 100.0f;
    float minCaribdisDistance = 100.0f;

    for (ObjectGuid const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || !unit->IsAlive())
            continue;

        float distance = bot->GetDistance(unit);
        
        if (unit->GetEntry() == NPC_FATHOM_GUARD_TIDALVESS && distance < minTidalvessDistance)
        {
            tidalvess = unit;
            minTidalvessDistance = distance;
        }
        else if (unit->GetEntry() == NPC_FATHOM_GUARD_SHARKKIS && distance < minSharkkisDistance)
        {
            sharkkis = unit;
            minSharkkisDistance = distance;
        }
        else if (unit->GetEntry() == NPC_FATHOM_GUARD_CARIBDIS && distance < minCaribdisDistance)
        {
            caribdis = unit;
            minCaribdisDistance = distance;
        }
    }

    // Priority order - kill Tidalvess first (totems), then Sharkkis (pets), then Caribdis (healer)
    Unit* target = nullptr;
    if (tidalvess)
    {
        target = tidalvess;
    }
    else if (sharkkis)
    {
        target = sharkkis;
    }
    else if (caribdis)
    {
        target = caribdis;
    }

    if (target)
    {
        if (AI_VALUE(Unit*, "current target") != target)
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(target);
        }
        return Attack(target);
    }

    return false;
}

bool KarathressTidalSurgeAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    // Check for Caribdis casting Tidal Surge
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
    {
        return false;
    }
    GuidVector npcs = npcsValue->Get();
    
    for (ObjectGuid const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_FATHOM_GUARD_CARIBDIS)
        {
            if (unit->FindCurrentSpellBySpellId(SPELL_KARATHRESS_TIDAL_SURGE))
            {
                float distance = bot->GetDistance(unit);
                if (distance < 10.0f) // Tidal Surge is a frontal cone stun
                {
                    Position movePos = bot->GetPosition();
                    float angle = bot->GetAngle(unit) + M_PI / 2; // Move to the side
                    movePos.m_positionX += cos(angle) * 8.0f;
                    movePos.m_positionY += sin(angle) * 8.0f;
                    
                    return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                                 false, false, false, true, MovementPriority::MOVEMENT_FORCED);
                }
            }
        }
    }

    return false;
}

bool KarathressTotemsAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
    {
        return false;
    }
    GuidVector npcs = npcsValue->Get();
    
    // Priority: Spitfire Totem > Earthbind Totem > Poison Cleansing Totem
    Unit* spitfire = nullptr;
    Unit* earthbind = nullptr;
    Unit* cleansing = nullptr;
    
    float minSpitfireDistance = 100.0f;
    float minEarthbindDistance = 100.0f;
    float minCleansingDistance = 100.0f;

    for (ObjectGuid const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || !unit->IsAlive())
            continue;

        float distance = bot->GetDistance(unit);
        
        if (unit->GetEntry() == NPC_SPITFIRE_TOTEM && distance < minSpitfireDistance)
        {
            spitfire = unit;
            minSpitfireDistance = distance;
        }
        else if (unit->GetEntry() == NPC_GREATER_EARTHBIND_TOTEM && distance < minEarthbindDistance)
        {
            earthbind = unit;
            minEarthbindDistance = distance;
        }
        else if (unit->GetEntry() == NPC_GREATER_POISON_CLEANSING_TOTEM && distance < minCleansingDistance)
        {
            cleansing = unit;
            minCleansingDistance = distance;
        }
    }

    // Kill priority: Spitfire (damage) > Earthbind (slow) > Cleansing
    Unit* target = nullptr;
    if (spitfire && minSpitfireDistance < 30.0f)
    {
        target = spitfire;
    }
    else if (earthbind && minEarthbindDistance < 30.0f)
    {
        target = earthbind;
    }
    else if (cleansing && minCleansingDistance < 30.0f)
    {
        target = cleansing;
    }

    if (target)
    {
        if (AI_VALUE(Unit*, "current target") != target)
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(target);
        }
        return Attack(target);
    }

    return false;
}

// Morogrim Tidewalker Actions
std::map<ObjectGuid, uint32> g_morogrim_lastGraveTime;

bool MorogrimTidalWaveAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Check if boss is casting Tidal Wave and bot is in front
    if (boss->FindCurrentSpellBySpellId(SPELL_MOROGRIM_TIDAL_WAVE) || boss->HasUnitState(UNIT_STATE_CASTING))
    {
        // Check if bot is in front of boss
        float angle = boss->GetAngle(bot);
        float facing = boss->GetOrientation();
        float diff = fabs(angle - facing);
        
        // Normalize angle difference
        if (diff > M_PI)
            diff = 2 * M_PI - diff;
            
        // In front cone (90 degrees)
        if (diff < M_PI / 4)
        {
            float distance = bot->GetDistance(boss);
            if (distance < 20.0f)
            {
                // Move behind boss
                Position movePos = boss->GetPosition();
                float moveAngle = facing + M_PI; // Behind boss
                movePos.m_positionX += cos(moveAngle) * 8.0f;
                movePos.m_positionY += sin(moveAngle) * 8.0f;
                
                return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                             false, false, false, true, MovementPriority::MOVEMENT_FORCED);
            }
        }
    }

    return false;
}

bool MorogrimWateryGraveAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    // Check if bot has Watery Grave debuff
    if (bot->HasAura(SPELL_MOROGRIM_WATERY_GRAVE_1) || 
        bot->HasAura(SPELL_MOROGRIM_WATERY_GRAVE_2) || 
        bot->HasAura(SPELL_MOROGRIM_WATERY_GRAVE_3) || 
        bot->HasAura(SPELL_MOROGRIM_WATERY_GRAVE_4))
    {
        ObjectGuid botGuid = bot->GetGUID();
        uint32 currentTime = getMSTime();
        
        // Track when we got grave'd for healing priority
        g_morogrim_lastGraveTime[botGuid] = currentTime;
        
        // Use healthstone if available and health is low
        if (bot->GetHealthPct() < 50.0f)
        {
            // This would trigger healthstone usage through normal healing logic
            return true;
        }
    }

    return false;
}

bool MorogrimMurlocsAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
    {
        return false;
    }
    GuidVector npcs = npcsValue->Get();
    
    Unit* closestMurloc = nullptr;
    float minDistance = 100.0f;

    for (ObjectGuid const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || !unit->IsAlive())
            continue;

        // Tidewalker Lurker murlocs
        if (unit->GetEntry() == NPC_TIDEWALKER_LURKER)
        {
            float distance = bot->GetDistance(unit);
            if (distance < minDistance)
            {
                closestMurloc = unit;
                minDistance = distance;
            }
        }
    }

    if (closestMurloc)
    {
        // Switch to murlocs for AOE
        if (AI_VALUE(Unit*, "current target") != closestMurloc)
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(closestMurloc);
        }
        
        // Move to stack point for AOE if tank
        if (botAI->IsTank(bot))
        {
            Unit* boss = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
            if (boss)
            {
                // Bring murlocs to boss for cleave
                Position stackPos = boss->GetPosition();
                float distance = bot->GetDistance(stackPos);
                if (distance > 5.0f)
                {
                    return MoveTo(bot->GetMapId(), stackPos.m_positionX, stackPos.m_positionY, stackPos.m_positionZ,
                                 false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
                }
            }
        }
        
        return Attack(closestMurloc);
    }

    return false;
}

bool MorogrimGlobulesAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Only in phase 2 (below 25%)
    if (boss->GetHealthPct() <= 25.0f)
    {
        Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
        if (!npcsValue)
        {
            return false;
        }
        GuidVector npcs = npcsValue->Get();
        
        Unit* closestGlobule = nullptr;
        float minDistance = 100.0f;

        for (ObjectGuid const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || !unit->IsAlive())
                continue;

            // Water Globule
            if (unit->GetEntry() == NPC_WATER_GLOBULE)
            {
                float distance = bot->GetDistance(unit);
                if (distance < minDistance)
                {
                    closestGlobule = unit;
                    minDistance = distance;
                }
            }
        }

        if (closestGlobule && minDistance < 15.0f)
        {
            // Kite away from globule
            Position movePos = bot->GetPosition();
            float angle = bot->GetAngle(closestGlobule) + M_PI;
            movePos.m_positionX += cos(angle) * 10.0f;
            movePos.m_positionY += sin(angle) * 10.0f;
            
            return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                         false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
    }

    return false;
}

bool MorogrimPositionAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    float desiredDistance;
    Position desiredPos;
    
    if (boss->GetHealthPct() <= 25.0f)
    {
        // Phase 2 - move to entrance, need room for globules
        desiredDistance = botAI->IsMelee(bot) ? 5.0f : 30.0f;
        
        // Position at entrance
        desiredPos = boss->GetPosition();
    }
    else
    {
        // Phase 1 - stack behind boss
        desiredDistance = botAI->IsMelee(bot) ? 5.0f : 15.0f;
        
        // Get behind boss
        float bossOrientation = boss->GetOrientation();
        desiredPos = boss->GetPosition();
        desiredPos.m_positionX += cos(bossOrientation + M_PI) * desiredDistance;
        desiredPos.m_positionY += sin(bossOrientation + M_PI) * desiredDistance;
    }

    float currentDistance = bot->GetDistance(desiredPos);
    if (currentDistance > 5.0f)
    {
        return MoveTo(bot->GetMapId(), desiredPos.m_positionX, desiredPos.m_positionY, desiredPos.m_positionZ,
                     false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

// Lady Vashj Actions
bool VashjShockBlastAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Check if boss is casting Shock Blast (frontal cone)
    if (boss->FindCurrentSpellBySpellId(SPELL_VASHJ_SHOCK_BLAST))
    {
        // Get behind or to the side of boss
        float angle = boss->GetOrientation() + M_PI;
        float distance = 10.0f;
        float x = boss->GetPositionX() + cos(angle) * distance;
        float y = boss->GetPositionY() + sin(angle) * distance;
        float z = boss->GetPositionZ();
        
        boss->UpdateAllowedPositionZ(x, y, z);
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }

    return false;
}

bool VashjStaticChargeAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    // Static Charge requires spreading
    if (bot->HasAura(SPELL_VASHJ_STATIC_CHARGE))
    {
        Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
        if (!membersValue)
        {
            return false;
        }
        GuidVector members = membersValue->Get();
        
        for (ObjectGuid const& member : members)
        {
            Unit* player = botAI->GetUnit(member);
            if (player && player != bot && player->IsAlive())
            {
                float distance = bot->GetDistance(player);
                if (distance < 10.0f)
                {
                    // Move away from nearby players
                    float angle = bot->GetAngle(player) + M_PI;
                    float moveDistance = 15.0f;
                    float x = bot->GetPositionX() + cos(angle) * moveDistance;
                    float y = bot->GetPositionY() + sin(angle) * moveDistance;
                    float z = bot->GetPositionZ();
                    
                    bot->UpdateAllowedPositionZ(x, y, z);
                    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
                }
            }
        }
    }

    return false;
}

bool VashjEntangleAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    // Check if entangled and need dispel/freedom
    if (bot->HasAura(SPELL_VASHJ_ENTANGLE))
    {
        // Request dispel or use freedom abilities
        Value<std::list<uint32>>* dispelSpellsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("dispel spell", "root");
        if (dispelSpellsValue)
        {
            std::list<uint32> dispelSpells = dispelSpellsValue->Get();
            for (uint32 spellId : dispelSpells)
            {
                if (botAI->CanCastSpell(spellId, bot))
                {
                    return botAI->CastSpell(spellId, bot);
                }
            }
        }
    }

    return false;
}

bool VashjEnchantedElementalAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Phase 2 - Enchanted Elementals must die quickly
    if (boss->HasAura(SPELL_VASHJ_MAGIC_BARRIER))
    {
        Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
        if (!npcsValue)
        {
            return false;
        }
        GuidVector npcs = npcsValue->Get();
        
        Unit* nearestElemental = nullptr;
        float minDistance = 100.0f;
        
        for (ObjectGuid const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->GetEntry() == NPC_ENCHANTED_ELEMENTAL)
            {
                float distance = bot->GetDistance(unit);
                if (distance < minDistance)
                {
                    minDistance = distance;
                    nearestElemental = unit;
                }
            }
        }
        
        if (nearestElemental)
        {
            if (AI_VALUE(Unit*, "current target") != nearestElemental)
            {
                botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nearestElemental);
            }
            return Attack(nearestElemental);
        }
    }

    return false;
}

bool VashjTaintedElementalAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Phase 2 - Tainted Elementals drop cores needed for shield generators
    if (boss->HasAura(SPELL_VASHJ_MAGIC_BARRIER))
    {
        Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
        if (!npcsValue)
        {
            return false;
        }
        GuidVector npcs = npcsValue->Get();
        
        Unit* nearestTainted = nullptr;
        float minDistance = 100.0f;
        
        for (ObjectGuid const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->GetEntry() == NPC_TAINTED_ELEMENTAL)
            {
                float distance = bot->GetDistance(unit);
                if (distance < minDistance)
                {
                    minDistance = distance;
                    nearestTainted = unit;
                }
            }
        }
        
        if (nearestTainted)
        {
            if (AI_VALUE(Unit*, "current target") != nearestTainted)
            {
                botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nearestTainted);
            }
            return Attack(nearestTainted);
        }
    }

    return false;
}

bool VashjCoilfangEliteAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Phase 2 - Coilfang Elites are high priority
    if (boss->HasAura(SPELL_VASHJ_MAGIC_BARRIER))
    {
        Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
        if (!npcsValue)
        {
            return false;
        }
        GuidVector npcs = npcsValue->Get();
        
        Unit* nearestElite = nullptr;
        float minDistance = 100.0f;
        
        for (ObjectGuid const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->GetEntry() == NPC_COILFANG_ELITE)
            {
                float distance = bot->GetDistance(unit);
                if (distance < minDistance)
                {
                    minDistance = distance;
                    nearestElite = unit;
                }
            }
        }
        
        if (nearestElite)
        {
            if (AI_VALUE(Unit*, "current target") != nearestElite)
            {
                botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nearestElite);
            }
            return Attack(nearestElite);
        }
    }

    return false;
}

bool VashjCoilfangStriderAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Phase 2 - Coilfang Striders fear and must be controlled
    if (boss->HasAura(SPELL_VASHJ_MAGIC_BARRIER))
    {
        Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
        if (!npcsValue)
        {
            return false;
        }
        GuidVector npcs = npcsValue->Get();
        
        Unit* nearestStrider = nullptr;
        float minDistance = 100.0f;
        
        for (ObjectGuid const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->GetEntry() == NPC_COILFANG_STRIDER)
            {
                float distance = bot->GetDistance(unit);
                if (distance < minDistance)
                {
                    minDistance = distance;
                    nearestStrider = unit;
                }
            }
        }
        
        if (nearestStrider)
        {
            // Kite striders if possible
            if (botAI->IsRanged(bot) && minDistance < 15.0f)
            {
                float angle = bot->GetAngle(nearestStrider) + M_PI;
                float moveDistance = 20.0f;
                float x = bot->GetPositionX() + cos(angle) * moveDistance;
                float y = bot->GetPositionY() + sin(angle) * moveDistance;
                float z = bot->GetPositionZ();
                
                bot->UpdateAllowedPositionZ(x, y, z);
                return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
            }
            
            if (AI_VALUE(Unit*, "current target") != nearestStrider)
            {
                botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nearestStrider);
            }
            return Attack(nearestStrider);
        }
    }

    return false;
}

bool VashjSporebatAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Phase 3 - Toxic Sporebats drop poison clouds
    if (!boss->HasAura(SPELL_VASHJ_MAGIC_BARRIER) && boss->GetHealthPct() <= 50.0f)
    {
        Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
        if (!npcsValue)
        {
            return false;
        }
        GuidVector npcs = npcsValue->Get();
        
        Unit* nearestBat = nullptr;
        float minDistance = 100.0f;
        
        for (ObjectGuid const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->GetEntry() == NPC_TOXIC_SPOREBAT)
            {
                float distance = bot->GetDistance(unit);
                if (distance < minDistance)
                {
                    minDistance = distance;
                    nearestBat = unit;
                }
            }
        }
        
        if (nearestBat)
        {
            // Ranged should prioritize bats
            if (botAI->IsRanged(bot))
            {
                if (AI_VALUE(Unit*, "current target") != nearestBat)
                {
                    botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nearestBat);
                }
                return Attack(nearestBat);
            }
            // Melee avoid getting close to bats
            else if (minDistance < 10.0f)
            {
                float angle = bot->GetAngle(nearestBat) + M_PI;
                float moveDistance = 15.0f;
                float x = bot->GetPositionX() + cos(angle) * moveDistance;
                float y = bot->GetPositionY() + sin(angle) * moveDistance;
                float z = bot->GetPositionZ();
                
                bot->UpdateAllowedPositionZ(x, y, z);
                return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
            }
        }
    }

    return false;
}

bool VashjPositionAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    float desiredDistance;
    
    // Phase 2 - spread out for adds
    if (boss->HasAura(SPELL_VASHJ_MAGIC_BARRIER))
    {
        desiredDistance = botAI->IsRanged(bot) ? 30.0f : 20.0f;
    }
    // Phase 1 and 3 - standard positioning
    else
    {
        desiredDistance = botAI->IsMelee(bot) ? 5.0f : 25.0f;
    }
    
    float currentDistance = bot->GetDistance(boss);
    if (fabs(currentDistance - desiredDistance) > 5.0f)
    {
        float angle = boss->GetAngle(bot);
        float x = boss->GetPositionX() + cos(angle) * desiredDistance;
        float y = boss->GetPositionY() + sin(angle) * desiredDistance;
        float z = boss->GetPositionZ();
        
        boss->UpdateAllowedPositionZ(x, y, z);
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool VashjTaintedCoreAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Phase 2 - check if bot has tainted core item
    if (boss->HasAura(SPELL_VASHJ_MAGIC_BARRIER))
    {
        // Check if bot has tainted core in inventory
        if (bot->HasItemCount(ITEM_TAINTED_CORE, 1))
        {
            // Move to shield generator position (simplified - would need actual generator positions)
            // This is a placeholder - actual implementation would need generator GameObject positions
            Position generatorPos(30.0f, -923.0f, 42.0f);
            float distance = bot->GetDistance(generatorPos);
            
            if (distance > 5.0f)
            {
                return MoveTo(bot->GetMapId(), generatorPos.GetPositionX(), generatorPos.GetPositionY(), 
                             generatorPos.GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_FORCED);
            }
        }
    }

    return false;
}