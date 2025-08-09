#include "MagistersTerraceActions.h"
#include "MagistersTerraceTriggers.h"
#include "SpellInfo.h"
#include "Unit.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
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

    Unit* boss = bot->FindNearestCreature(NPC_KAELTHAS, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_PYROBLAST))
    {
        // Use interrupt spell list
        std::list<uint32> spellIds = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "interrupt")->Get();
        for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
        {
            uint32 spellId = *it;
            if (botAI->CanCastSpell(spellId, boss, false))
            {
                return botAI->CastSpell(spellId, boss);
            }
        }
    }
    return false;
}

bool InterruptKaelthasPyroblastAction::isUseful()
{
    return AI_VALUE(bool, "kaelthas casting pyroblast");
}

bool AvoidGravityLapseAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    uint32 currentTime = getMSTime();

    // Check if boss is casting gravity lapse
    Unit* boss = bot->FindNearestCreature(NPC_KAELTHAS, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Reset state if needed (new gravity lapse phase)
    if (g_kaelthas_inSafePosition[botGuid])
    {
        if ((currentTime - g_kaelthas_lastMoveTime[botGuid]) > 10000)
        {
            g_kaelthas_inSafePosition[botGuid] = false;
        }
        else
        {
            return false; // Already in safe position
        }
    }

    // Move away from center during gravity lapse
    Position centerPos(225.0f, -5.0f, -2.0f); // Center of room
    float distance = bot->GetDistance(centerPos);
    
    if (distance < 20.0f)
    {
        if (FleePosition(centerPos, 25.0f, 500U))
        {
            g_kaelthas_lastMoveTime[botGuid] = currentTime;
            g_kaelthas_inSafePosition[botGuid] = true;
            return true;
        }
    }

    return false;
}

bool AvoidGravityLapseAction::isUseful()
{
    return AI_VALUE(bool, "kaelthas casting gravity lapse");
}

bool FleeArcaneSphereAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find nearest arcane sphere
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 15.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 15.0f);

    Unit* closestSphere = nullptr;
    float closestDistance = 15.0f;

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
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
    return AI_VALUE(bool, "kaelthas arcane sphere nearby");
}

// Vexallus Actions
bool AttackPureEnergyAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find Pure Energy adds
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 50.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 50.0f);

    Unit* pureEnergy = nullptr;
    float closestDistance = 50.0f;

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
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
    return AI_VALUE(bool, "vexallus pure energy spawned");
}

// Selin Fireheart Actions
bool AvoidFelExplosionAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_SELIN_FIREHEART, 100.0f);
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
    return AI_VALUE(bool, "selin fireheart fel explosion");
}

bool AttackFelCrystalAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Selin is channeling
    Unit* boss = bot->FindNearestCreature(NPC_SELIN_FIREHEART, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    if (!(boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_MANA_RAGE)))
        return false;

    // Find the crystal being channeled
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 50.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 50.0f);

    Unit* crystal = nullptr;
    float closestDistance = 50.0f;

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
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
    return AI_VALUE(bool, "fel crystal nearby");
}

// Delrissa Actions
bool AttackDelrissaAddAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Delrissa is in combat
    Unit* boss = bot->FindNearestCreature(NPC_DELRISSA, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // List of possible add IDs
    const uint32 delrissaAdds[] = {24557, 24558, 24554, 24561, 24559, 24555, 24553, 24556};

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 50.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 50.0f);

    Unit* priorityTarget = nullptr;
    float closestDistance = 50.0f;

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
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
    return AI_VALUE(bool, "delrissa add active");
}