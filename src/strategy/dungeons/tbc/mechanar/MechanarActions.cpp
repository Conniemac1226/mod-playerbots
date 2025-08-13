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

    // RESEARCHED: boss_nethermancer_sepethrea.cpp:146-157
    // Raging Flames fixate on random targets and re-fixate every 15-25s during Inferno
    // ALL players must move away from elementals to avoid AoE damage
    
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* dangerousElement = nullptr;
    float closestDanger = 100.0f;
    bool isDirectlyFixated = false;
    
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_RAGING_FLAMES)
        {
            float distance = bot->GetDistance(unit);
            bool isCastingInferno = unit->HasUnitState(UNIT_STATE_CASTING) && 
                                  unit->FindCurrentSpellBySpellId(SPELL_INFERNO);
            
            // CRITICAL: Move away from Inferno casting (immediate AoE danger)
            if (isCastingInferno && distance < 15.0f)
            {
                dangerousElement = unit;
                closestDanger = distance;
                break; // Immediate action needed
            }
            
            // HIGH PRIORITY: Directly fixated by this elemental
            if (unit->GetVictim() == bot)
            {
                dangerousElement = unit;
                closestDanger = distance;
                isDirectlyFixated = true;
                g_sepethrea_targetedByFlames[botGuid] = unit->GetGUID();
                g_sepethrea_lastFlamesTime[botGuid] = currentTime;
            }
            
            // MODERATE PRIORITY: Close enough to be dangerous
            else if (!dangerousElement && distance < 18.0f)
            {
                if (distance < closestDanger)
                {
                    dangerousElement = unit;
                    closestDanger = distance;
                }
            }
        }
    }

    if (dangerousElement)
    {
        // Calculate safe escape position
        float angle = bot->GetAngle(dangerousElement) + M_PI; // Opposite direction
        float escapeDistance = isDirectlyFixated ? 25.0f : 20.0f; // Kite further if fixated
        
        // Try to move towards raid area if possible, otherwise just away
        Position safePos;
        safePos.m_positionX = bot->GetPositionX() + cos(angle) * escapeDistance;
        safePos.m_positionY = bot->GetPositionY() + sin(angle) * escapeDistance;
        safePos.m_positionZ = bot->GetPositionZ();
        
        return MoveTo(bot->GetMapId(), safePos.m_positionX, safePos.m_positionY,
                     safePos.m_positionZ, false, false, false, true,
                     MovementPriority::MOVEMENT_FORCED);
    }

    // Clear targeting if flames despawned or retargeted
    if (g_sepethrea_targetedByFlames[botGuid])
    {
        if ((currentTime - g_sepethrea_lastFlamesTime[botGuid]) > 10000)
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
    // Dragon's Breath is a frontal cone
    if (boss->HasUnitState(UNIT_STATE_CASTING) && 
        boss->FindCurrentSpellBySpellId(SPELL_DRAGONS_BREATH))
    {
        if (boss->HasInArc(M_PI / 3, bot) && !botAI->IsTank(bot))
        {
            // Move to the side
            float angle = boss->GetOrientation() + (M_PI / 2);
            Position safePos = boss->GetPosition();
            safePos.m_positionX += cos(angle) * 10.0f;
            safePos.m_positionY += sin(angle) * 10.0f;
            
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
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: boss_nethermancer_sepethrea.cpp:146-157
    // Raging Flames must be killed quickly to reduce raid damage
    // DPS should prioritize elementals over boss
    
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* priorityElemental = nullptr;
    float closestDistance = 100.0f;
    
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_RAGING_FLAMES)
        {
            float distance = bot->GetDistance(unit);
            
            // Prioritize elementals that are:
            // 1. Casting Inferno (most dangerous)
            // 2. Closest to raid members
            // 3. Fixated on healers/casters (protect them)
            
            bool isCastingInferno = unit->HasUnitState(UNIT_STATE_CASTING) && 
                                  unit->FindCurrentSpellBySpellId(SPELL_INFERNO);
            bool fixatingHealer = false;
            
            if (unit->GetVictim() && unit->GetVictim()->IsPlayer())
            {
                Player* victim = unit->GetVictim()->ToPlayer();
                fixatingHealer = botAI->IsHeal(victim) || !botAI->IsMelee(victim);
            }
            
            // Calculate priority score (lower = higher priority)
            float priority = distance;
            if (isCastingInferno) priority -= 50.0f; // Highest priority
            if (fixatingHealer) priority -= 20.0f;   // Protect healers/casters
            
            if (priority < closestDistance && distance <= 40.0f)
            {
                priorityElemental = unit;
                closestDistance = priority;
            }
        }
    }

    if (priorityElemental)
    {
        // Switch target to the priority elemental
        Value<Unit*>* currentTargetValue = botAI->GetAiObjectContext()->GetValue<Unit*>("current target");
        if (currentTargetValue && currentTargetValue->Get() != priorityElemental)
        {
            currentTargetValue->Set(priorityElemental);
        }
        return Attack(priorityElemental);
    }

    return false;
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