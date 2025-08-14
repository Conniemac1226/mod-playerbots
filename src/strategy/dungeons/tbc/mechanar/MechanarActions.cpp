#include "MechanarActions.h"
#include "MechanarTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// Per-bot state management
std::map<ObjectGuid, uint32> g_capacitus_lastPolarityTime;
std::map<ObjectGuid, bool> g_capacitus_hasPositive;
std::map<ObjectGuid, bool> g_capacitus_hasNegative;

std::map<ObjectGuid, ObjectGuid> g_sepethrea_targetedByFlames;
std::map<ObjectGuid, uint32> g_sepethrea_lastFlamesTime;

// ========== MECHANO LORD CAPACITUS ACTIONS ==========

bool CapacitusReflectiveShieldAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "mechano lord capacitus");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: boss_mechano_lord_capacitus.cpp:89-96
    // Normal mode: alternates between magic and damage shields
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

    // RESEARCHED: boss_mechano_lord_capacitus.cpp:78-82 & 135-217
    // Polarity Shift gives players positive or negative charge
    // Players with same charge need to stack, opposite charges must spread
    
    bool hasPositive = bot->HasAura(SPELL_POSITIVE_POLARITY);
    bool hasNegative = bot->HasAura(SPELL_NEGATIVE_POLARITY);
    
    if (!hasPositive && !hasNegative)
    {
        g_capacitus_hasPositive[botGuid] = false;
        g_capacitus_hasNegative[botGuid] = false;
        return false;
    }

    // Update polarity state
    g_capacitus_hasPositive[botGuid] = hasPositive;
    g_capacitus_hasNegative[botGuid] = hasNegative;
    g_capacitus_lastPolarityTime[botGuid] = currentTime;

    // Find other players with same/opposite polarity
    const GuidVector members = AI_VALUE(GuidVector, "group members");
    Position safePos;
    bool foundSamePolarity = false;
    
    for (auto& member : members)
    {
        Unit* ally = botAI->GetUnit(member);
        if (!ally || ally == bot || !ally->IsAlive())
            continue;

        bool allyPositive = ally->HasAura(SPELL_POSITIVE_POLARITY);
        bool allyNegative = ally->HasAura(SPELL_NEGATIVE_POLARITY);
        
        if ((hasPositive && allyPositive) || (hasNegative && allyNegative))
        {
            // Same polarity - move closer to stack
            float distance = bot->GetDistance(ally);
            if (distance > 5.0f)
            {
                safePos = ally->GetPosition();
                foundSamePolarity = true;
                break;
            }
        }
        else if ((hasPositive && allyNegative) || (hasNegative && allyPositive))
        {
            // Opposite polarity - stay away
            float distance = bot->GetDistance(ally);
            if (distance < 10.0f)
            {
                // EMERGENCY: Move away from opposite polarity
                float angle = bot->GetAngle(ally) + M_PI;
                float x = bot->GetPositionX() + cos(angle) * 15.0f;
                float y = bot->GetPositionY() + sin(angle) * 15.0f;
                float z = bot->GetPositionZ();
                return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                            MovementPriority::MOVEMENT_FORCED);
            }
        }
    }

    if (foundSamePolarity)
    {
        return MoveTo(bot->GetMapId(), safePos.m_positionX, safePos.m_positionY,
                     safePos.m_positionZ, false, false, false, true,
                     MovementPriority::MOVEMENT_FORCED);
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

    // RESEARCHED: boss_mechano_lord_capacitus.cpp:64-70
    // Nether Charges spawn and move randomly
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

    // Standard positioning - spread for charges
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
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                        MovementPriority::MOVEMENT_FORCED);
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

    ObjectGuid botGuid = bot->GetGUID();
    uint32 currentTime = getMSTime();

    // RESEARCHED: Raging Flames are IMMUNE to damage and must be KITED
    // They fixate on random targets and re-target every 8-13 seconds during Inferno
    // Strategy: Continuous kiting, maintain safe distance, avoid Inferno AoE
    
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* mostDangerousFlame = nullptr;
    float highestThreat = -1.0f;
    bool needsEmergencyMove = false;
    
    for (auto& npc : npcs)
    {
        Unit* flame = botAI->GetUnit(npc);
        if (!flame || !flame->IsAlive() || flame->GetEntry() != NPC_RAGING_FLAMES)
            continue;

        float distance = bot->GetDistance(flame);
        float threatLevel = 0.0f;
        
        // CRITICAL: Inferno is casting - EMERGENCY EVACUATION
        bool isCastingInferno = flame->HasUnitState(UNIT_STATE_CASTING) && 
                               flame->FindCurrentSpellBySpellId(SPELL_INFERNO);
        if (isCastingInferno && distance < 20.0f) // Increased AoE safety range
        {
            threatLevel = 1000.0f + (20.0f - distance) * 10.0f; // Closer = more urgent
            needsEmergencyMove = true;
        }
        // HIGH: Flame is fixated on this bot
        else if (flame->GetVictim() == bot)
        {
            threatLevel = 500.0f + (35.0f - distance); // Extended threat range
            g_sepethrea_targetedByFlames[botGuid] = flame->GetGUID();
            g_sepethrea_lastFlamesTime[botGuid] = currentTime;
        }
        // MODERATE: Flame is too close for comfort (extended detection)
        else if (distance < 25.0f) // Increased detection range
        {
            threatLevel = 100.0f + (25.0f - distance);
        }
        
        if (threatLevel > highestThreat)
        {
            mostDangerousFlame = flame;
            highestThreat = threatLevel;
        }
    }

    if (mostDangerousFlame && highestThreat > 0.0f)
    {
        // IMPROVED KITING ALGORITHM
        float currentDistance = bot->GetDistance(mostDangerousFlame);
        
        // Calculate optimal kiting position
        float flameAngle = bot->GetAngle(mostDangerousFlame);
        float escapeAngle = flameAngle + M_PI; // Opposite direction
        
        // Dynamic distance calculation based on threat level
        float safeDistance;
        if (needsEmergencyMove)
        {
            safeDistance = 35.0f; // Far from Inferno AoE
        }
        else if (mostDangerousFlame->GetVictim() == bot)
        {
            safeDistance = 22.0f; // Continuous kiting distance
        }
        else
        {
            safeDistance = 18.0f; // Safe proximity
        }
        
        // Add some randomization to avoid predictable movement patterns
        float angleVariation = (rand() % 60 - 30) * M_PI / 180.0f; // ±30 degrees
        escapeAngle += angleVariation;
        
        // Calculate destination with obstacle avoidance consideration
        Position kitePos;
        kitePos.m_positionX = bot->GetPositionX() + cos(escapeAngle) * safeDistance;
        kitePos.m_positionY = bot->GetPositionY() + sin(escapeAngle) * safeDistance;
        kitePos.m_positionZ = bot->GetPositionZ();
        
        // Use appropriate movement priority
        MovementPriority priority = needsEmergencyMove ? 
            MovementPriority::MOVEMENT_FORCED : MovementPriority::MOVEMENT_NORMAL;
        
        return MoveTo(bot->GetMapId(), kitePos.m_positionX, kitePos.m_positionY,
                     kitePos.m_positionZ, false, false, false, true, priority);
    }

    // Clean up old fixation tracking (aligned with 8-13s Inferno cycle)
    if (g_sepethrea_targetedByFlames[botGuid])
    {
        if ((currentTime - g_sepethrea_lastFlamesTime[botGuid]) > 15000) // 15s timeout
        {
            g_sepethrea_targetedByFlames[botGuid].Clear();
        }
    }

    return false;
}

bool SepethreaRagingFlamesAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* flamesValue = botAI->GetAiObjectContext()->GetValue<bool>("raging flames active");
    if (!flamesValue)
        return false;
    
    return flamesValue->Get();
}

bool SepethreaDragonsBreathAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "nethermancer sepethrea");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: boss_nethermancer_sepethrea.cpp:72
    // Dragon's Breath is a frontal cone - IMMEDIATE reaction needed
    if (boss->HasUnitState(UNIT_STATE_CASTING) && 
        boss->FindCurrentSpellBySpellId(SPELL_DRAGONS_BREATH))
    {
        // Check if we're in danger zone (frontal cone)
        if (boss->HasInArc(M_PI / 2, bot) && !botAI->IsTank(bot)) // Wider check for safety
        {
            // Calculate perpendicular escape direction (to the sides)
            float bossOrientation = boss->GetOrientation();
            float botAngleToBoss = boss->GetAngle(bot);
            
            // Determine which side is safer (left or right)
            float leftAngle = bossOrientation - (M_PI / 2);
            float rightAngle = bossOrientation + (M_PI / 2);
            
            // Choose the side that's closer to our current position
            float angleDiffLeft = fabs(botAngleToBoss - leftAngle);
            float angleDiffRight = fabs(botAngleToBoss - rightAngle);
            
            float escapeAngle = (angleDiffLeft < angleDiffRight) ? leftAngle : rightAngle;
            
            // Move to a safe position at the boss's side
            float escapeDistance = 15.0f; // Safe distance from boss
            Position safePos;
            safePos.m_positionX = boss->GetPositionX() + cos(escapeAngle) * escapeDistance;
            safePos.m_positionY = boss->GetPositionY() + sin(escapeAngle) * escapeDistance;
            safePos.m_positionZ = boss->GetPositionZ();
            
            return MoveTo(bot->GetMapId(), safePos.m_positionX, safePos.m_positionY,
                         safePos.m_positionZ, false, false, false, true,
                         MovementPriority::MOVEMENT_FORCED);
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

bool SepethreaInfernoAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: boss_nethermancer_sepethrea.cpp:153 & 187
    // Raging Flames cast Inferno which deals AoE damage
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_RAGING_FLAMES)
        {
            if (unit->HasAura(SPELL_INFERNO) || 
                (unit->HasUnitState(UNIT_STATE_CASTING) && 
                 unit->FindCurrentSpellBySpellId(SPELL_INFERNO)))
            {
                float distance = bot->GetDistance(unit);
                if (distance < 15.0f)
                {
                    // EMERGENCY: Move away from Inferno AoE
                    float angle = bot->GetAngle(unit) + M_PI;
                    float x = bot->GetPositionX() + cos(angle) * 20.0f;
                    float y = bot->GetPositionY() + sin(angle) * 20.0f;
                    float z = bot->GetPositionZ();
                    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                                MovementPriority::MOVEMENT_FORCED);
                }
            }
        }
    }

    return false;
}

bool SepethreaInfernoAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* infernoValue = botAI->GetAiObjectContext()->GetValue<bool>("inferno danger");
    if (!infernoValue)
        return false;
    
    return infernoValue->Get();
}

bool SepethreaArcaneBlastAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "nethermancer sepethrea");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: boss_nethermancer_sepethrea.cpp:63-68
    // Arcane Blast reduces threat by 50% on current target
    // Tanks need to be ready to taunt back
    if (botAI->IsTank(bot))
    {
        if (boss->GetVictim() && boss->GetVictim() != bot)
        {
            // Check if boss just cast Arcane Blast
            if (boss->HasUnitState(UNIT_STATE_CASTING) && 
                boss->FindCurrentSpellBySpellId(SPELL_ARCANE_BLAST))
            {
                // Be ready to taunt after cast
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
    // CRITICAL FIX: Raging Flames are IMMUNE to player damage!
    // This action should NOT try to attack them - they must be kited only
    // Removing all DPS targeting logic for Raging Flames
    
    // Instead, focus DPS back on the boss while flames are being kited
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "nethermancer sepethrea");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // Ensure we're targeting the boss instead of trying to fight immune flames
    Value<Unit*>* currentTargetValue = botAI->GetAiObjectContext()->GetValue<Unit*>("current target");
    if (currentTargetValue && currentTargetValue->Get() != boss)
    {
        currentTargetValue->Set(boss);
    }
    
    return Attack(boss);
}

bool SepethreaTargetElementalAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || botAI->IsTank(bot) || botAI->IsHeal(bot))
        return false; // Only for DPS
    
    Value<bool>* flamesValue = botAI->GetAiObjectContext()->GetValue<bool>("raging flames active");
    if (!flamesValue)
        return false;
    
    return flamesValue->Get();
}

// ========== PATHALEON THE CALCULATOR ACTIONS ==========

bool PathaleonDominationAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: boss_pathaleon_the_calculator.cpp:117-122
    // Domination is mind control - affected players need to be CC'd
    const GuidVector members = AI_VALUE(GuidVector, "group members");
    
    for (auto& member : members)
    {
        Unit* ally = botAI->GetUnit(member);
        if (!ally || ally == bot || !ally->IsAlive())
            continue;

        if (ally->HasAura(SPELL_DOMINATION))
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
    if (bot->HasAura(SPELL_DOMINATION))
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

    // RESEARCHED: boss_pathaleon_the_calculator.cpp:96-101
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

    // RESEARCHED: boss_pathaleon_the_calculator.cpp:113-115
    // Arcane Torrent is an AoE around the boss
    if (boss->HasUnitState(UNIT_STATE_CASTING) && 
        boss->FindCurrentSpellBySpellId(SPELL_ARCANE_TORRENT))
    {
        float distance = bot->GetDistance(boss);
        if (distance < 20.0f && !botAI->IsMelee(bot))
        {
            // EMERGENCY: Move out of Arcane Torrent range
            float angle = bot->GetAngle(boss) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 25.0f;
            float y = bot->GetPositionY() + sin(angle) * 25.0f;
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                        MovementPriority::MOVEMENT_FORCED);
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

    // RESEARCHED: boss_pathaleon_the_calculator.cpp:84-90
    // At 20% health: Frenzy + Suicide (explodes on death)
    if (boss->HasAura(SPELL_FRENZY))
    {
        // Boss will explode on death - prepare to run
        if (boss->GetHealthPct() < 5.0f)
        {
            // Get ready to flee when he dies
            // EMERGENCY: Get ready to flee when he dies (suicide explosion)
            float angle = bot->GetAngle(boss) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 30.0f;
            float y = bot->GetPositionY() + sin(angle) * 30.0f;
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                        MovementPriority::MOVEMENT_FORCED);
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
    return boss && boss->HasAura(SPELL_FRENZY);
}

bool PathaleonArcaneExplosionAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot || !bot->GetMap()->IsHeroic())
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "pathaleon the calculator");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: boss_pathaleon_the_calculator.cpp:131-135
    // Arcane Explosion on Heroic - AoE around boss
    if (boss->HasUnitState(UNIT_STATE_CASTING) && 
        boss->FindCurrentSpellBySpellId(SPELL_ARCANE_EXPLOSION))
    {
        float distance = bot->GetDistance(boss);
        if (distance < 10.0f && !botAI->IsMelee(bot))
        {
            // EMERGENCY: Move out of Arcane Explosion range
            float angle = bot->GetAngle(boss) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 15.0f;
            float y = bot->GetPositionY() + sin(angle) * 15.0f;
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                        MovementPriority::MOVEMENT_FORCED);
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

    // RESEARCHED: boss_pathaleon_the_calculator.cpp:104-109
    // Mana Tap drains mana from a random player
    if (bot->HasAura(SPELL_MANA_TAP))
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
    return bot && bot->getPowerType() == POWER_MANA && bot->HasAura(SPELL_MANA_TAP);
}