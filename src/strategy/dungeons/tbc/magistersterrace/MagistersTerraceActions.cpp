#include "MagistersTerraceActions.h"
#include "MagistersTerraceTriggers.h"
#include "SpellInfo.h"
#include "Unit.h"
#include "AttackersValue.h"
#include "Playerbots.h"

// Per-bot state maps for Kael'thas gravity lapse
std::map<ObjectGuid, uint32> g_kaelthas_lastMoveTime;
std::map<ObjectGuid, bool> g_kaelthas_inSafePosition;

// Kael'thas Actions
bool InterruptKaelthasPyroblastAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_PYROBLAST))
    {
        // Use interrupt spell list - SAFE PATTERN
        Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "interrupt");
        if (spellIdsValue)
        {
            std::list<uint32> spellIds = spellIdsValue->Get();
            for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
            {
                uint32 spellId = *it;
                if (botAI->CanCastSpell(spellId, boss, false))
                {
                    return botAI->CastSpell(spellId, boss);
                }
            }
        }
    }
    return false;
}

bool InterruptKaelthasPyroblastAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* pyroblastValue = botAI->GetAiObjectContext()->GetValue<bool>("kaelthas casting pyroblast");
    if (!pyroblastValue)
        return false;
    
    return pyroblastValue->Get();
}

bool AvoidGravityLapseAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    uint32 currentTime = getMSTime();

    // Check if boss is casting or channeling gravity lapse
    Unit* boss = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // CRITICAL: Interrupt any casting immediately during gravity lapse
    if (bot->IsNonMeleeSpellCast(false))
    {
        bot->InterruptNonMeleeSpells(true);
        return true; // Priority: Stop casting first
    }

    Position centerPos(225.0f, -5.0f, -2.0f); // Center of room
    float distance = bot->GetDistance(centerPos);
    
    // CONTINUOUS POSITIONING: Always maintain safe distance during gravity lapse
    if (distance < 25.0f) // Increased safety radius
    {
        // Reset state tracking for continuous movement
        g_kaelthas_inSafePosition[botGuid] = false;
        
        // Calculate escape position - move directly away from center
        float angle = centerPos.GetAngle(bot);
        float safeDistance = 30.0f; // Far from center
        
        float safeX = centerPos.GetPositionX() + cos(angle) * safeDistance;
        float safeY = centerPos.GetPositionY() + sin(angle) * safeDistance;
        float safeZ = centerPos.GetPositionZ();
        
        // Force movement with highest priority
        bool moved = MoveTo(bot->GetMapId(), safeX, safeY, safeZ, 
                           false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        
        if (moved)
        {
            g_kaelthas_lastMoveTime[botGuid] = currentTime;
            return true;
        }
        
        // Fallback: Use FleePosition if MoveTo fails
        if (FleePosition(centerPos, 30.0f, 100U))
        {
            g_kaelthas_lastMoveTime[botGuid] = currentTime;
            return true;
        }
    }
    
    // Even at safe distance, prevent casting and maintain positioning
    if (distance < 35.0f)
    {
        // Update timer to show we're actively managing this
        g_kaelthas_lastMoveTime[botGuid] = currentTime;
        
        // Don't allow any spellcasting during gravity lapse
        if (bot->IsNonMeleeSpellCast(false))
        {
            bot->InterruptNonMeleeSpells(true);
            return true;
        }
        
        // Keep moving slightly to avoid getting pulled back
        if ((currentTime - g_kaelthas_lastMoveTime[botGuid]) > 2000) // Every 2 seconds
        {
            float angle = frand(0, 2 * M_PI);
            float adjustX = bot->GetPositionX() + cos(angle) * 3.0f;
            float adjustY = bot->GetPositionY() + sin(angle) * 3.0f;
            float adjustZ = bot->GetPositionZ();
            
            MoveTo(bot->GetMapId(), adjustX, adjustY, adjustZ, 
                  false, false, false, true, MovementPriority::MOVEMENT_FORCED);
            return true;
        }
    }

    return false;
}

bool AvoidGravityLapseAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* gravityLapseValue = botAI->GetAiObjectContext()->GetValue<bool>("kaelthas casting gravity lapse");
    if (!gravityLapseValue)
        return false;
    
    return gravityLapseValue->Get();
}

bool FleeArcaneSphereAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find nearest arcane sphere using proven pattern
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* closestSphere = nullptr;
    float closestDistance = 15.0f;

    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_ARCANE_SPHERE)
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                closestSphere = unit;
                closestDistance = distance;
            }
        }
    }

    if (closestSphere && closestDistance < 10.0f)
    {
        return FleePosition(closestSphere->GetPosition(), 15.0f, 500U);
    }

    return false;
}

bool FleeArcaneSphereAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* sphereValue = botAI->GetAiObjectContext()->GetValue<bool>("kaelthas arcane sphere nearby");
    if (!sphereValue)
        return false;
    
    return sphereValue->Get();
}

// Vexallus Actions
bool AttackPureEnergyAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find Pure Energy adds using proven pattern
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* pureEnergy = nullptr;
    float closestDistance = 50.0f;

    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_PURE_ENERGY && AttackersValue::IsValidTarget(unit, bot))
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                pureEnergy = unit;
                closestDistance = distance;
            }
        }
    }

    if (pureEnergy)
    {
        return Attack(pureEnergy);
    }

    return false;
}

bool AttackPureEnergyAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    // Use safe Value pattern to prevent crashes
    Value<bool>* pureEnergyValue = botAI->GetAiObjectContext()->GetValue<bool>("vexallus pure energy spawned");
    if (!pureEnergyValue)
        return false;
    
    return pureEnergyValue->Get();
}

// Selin Fireheart Actions
bool AvoidFelExplosionAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "selin fireheart");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Move away during Fel Explosion cast
    float distance = bot->GetDistance(boss);
    if (distance < 15.0f)
    {
        return FleePosition(boss->GetPosition(), 20.0f, 500U);
    }

    return false;
}

bool AvoidFelExplosionAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* explosionValue = botAI->GetAiObjectContext()->GetValue<bool>("selin fireheart fel explosion");
    if (!explosionValue)
        return false;
    
    return explosionValue->Get();
}

bool AttackFelCrystalAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Selin is channeling
    Unit* boss = AI_VALUE2(Unit*, "find target", "selin fireheart");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    if (!(boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_MANA_RAGE)))
        return false;

    // Find the crystal being channeled using proven pattern
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* crystal = nullptr;
    float closestDistance = 50.0f;

    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        // Crystal becomes selectable when being channeled
        if (unit->GetEntry() == NPC_FEL_CRYSTAL && !unit->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                crystal = unit;
                closestDistance = distance;
            }
        }
    }

    if (crystal)
    {
        return Attack(crystal);
    }

    return false;
}

bool AttackFelCrystalAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* crystalValue = botAI->GetAiObjectContext()->GetValue<bool>("fel crystal nearby");
    if (!crystalValue)
        return false;
    
    return crystalValue->Get();
}

// Delrissa Actions
bool AttackDelrissaAddAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Delrissa is in combat
    Unit* boss = AI_VALUE2(Unit*, "find target", "priestess delrissa");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // List of possible add IDs
    const uint32 delrissaAdds[] = {24557, 24558, 24554, 24561, 24559, 24555, 24553, 24556};

    // Find adds using proven pattern
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* priorityTarget = nullptr;
    float closestDistance = 50.0f;

    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        for (uint32 addId : delrissaAdds)
        {
            if (unit->GetEntry() == addId && unit->IsInCombat() && AttackersValue::IsValidTarget(unit, bot))
            {
                float distance = bot->GetDistance(unit);
                if (distance < closestDistance)
                {
                    priorityTarget = unit;
                    closestDistance = distance;
                }
                break;
            }
        }
    }

    if (priorityTarget)
    {
        return Attack(priorityTarget);
    }

    return false;
}

bool AttackDelrissaAddAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Value<bool>* addActiveValue = botAI->GetAiObjectContext()->GetValue<bool>("delrissa add active");
    if (!addActiveValue)
        return false;
    
    return addActiveValue->Get();
}