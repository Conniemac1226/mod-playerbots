#include "SteamvaultActions.h"
#include "SteamvaultTriggers.h"
#include "SpellInfo.h"
#include "Unit.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "AttackersValue.h"
#include "Playerbots.h"

// Per-bot state maps for Lightning Cloud avoidance
std::map<ObjectGuid, uint32> g_thespia_lastMoveTime;
std::map<ObjectGuid, bool> g_thespia_inSafePosition;

// Hydromancer Thespia Actions
bool AvoidLightningCloudAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    ObjectGuid botGuid = bot->GetGUID();
    uint32 currentTime = getMSTime();

    Unit* boss = bot->FindNearestCreature(NPC_HYDROMANCER_THESPIA, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Reset state if needed (new lightning cloud cast)
    if (g_thespia_inSafePosition[botGuid])
    {
        if ((currentTime - g_thespia_lastMoveTime[botGuid]) > 8000)
        {
            g_thespia_inSafePosition[botGuid] = false;
        }
        else
        {
            return false; // Already moved
        }
    }

    // Move away from boss during Lightning Cloud
    float distance = bot->GetDistance(boss);
    if (distance < 20.0f)
    {
        if (FleePosition(boss->GetPosition(), 25.0f, 500U))
        {
            g_thespia_lastMoveTime[botGuid] = currentTime;
            g_thespia_inSafePosition[botGuid] = true;
            return true;
        }
    }

    return false;
}

bool AvoidLightningCloudAction::isUseful()
{
    return AI_VALUE(bool, "thespia lightning cloud");
}

bool DispelLungBurstAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Try to dispel Lung Burst from self or allies
    if (bot->HasAura(SPELL_LUNG_BURST))
    {
        std::list<uint32> spellIds = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "dispel")->Get();
        for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
        {
            uint32 spellId = *it;
            if (botAI->CanCastSpell(spellId, bot, false))
            {
                return botAI->CastSpell(spellId, bot);
            }
        }
    }
    return false;
}

bool DispelLungBurstAction::isUseful()
{
    return AI_VALUE(bool, "thespia lung burst");
}

bool AttackWaterElementalAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find water elementals
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 50.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 50.0f);

    Unit* elemental = nullptr;
    float closestDistance = 50.0f;

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_THESPIA_WATER_ELEMENTAL && AttackersValue::IsValidTarget(unit, bot))
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                elemental = unit;
                closestDistance = distance;
            }
        }
    }

    if (elemental)
    {
        return Attack(elemental);
    }

    return false;
}

bool AttackWaterElementalAction::isUseful()
{
    return AI_VALUE(bool, "thespia water elemental active");
}

// Mekgineer Steamrigger Actions
bool DispelShrinkRayAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Try to dispel shrink ray
    if (bot->HasAura(SPELL_SUPER_SHRINK_RAY))
    {
        std::list<uint32> spellIds = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "dispel")->Get();
        for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
        {
            uint32 spellId = *it;
            if (botAI->CanCastSpell(spellId, bot, false))
            {
                return botAI->CastSpell(spellId, bot);
            }
        }
    }
    return false;
}

bool DispelShrinkRayAction::isUseful()
{
    return AI_VALUE(bool, "steamrigger shrink ray");
}

bool AvoidSawBladeAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_MEKGINEER_STEAMRIGGER, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Move perpendicular to boss during Saw Blade cast
    float angle = bot->GetAngle(boss);
    float newAngle = angle + M_PI / 2; // Move 90 degrees to the side
    
    float moveX = bot->GetPositionX() + cos(newAngle) * 10.0f;
    float moveY = bot->GetPositionY() + sin(newAngle) * 10.0f;
    float moveZ = bot->GetPositionZ();

    return MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
}

bool AvoidSawBladeAction::isUseful()
{
    return AI_VALUE(bool, "steamrigger saw blade");
}

bool RemoveElectrifiedNetAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Try to remove net with escape abilities
    if (bot->HasAura(SPELL_ELECTRIFIED_NET))
    {
        // Try PvP trinket or escape abilities
        std::list<uint32> spellIds = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "escape")->Get();
        for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
        {
            uint32 spellId = *it;
            if (botAI->CanCastSpell(spellId, bot, false))
            {
                return botAI->CastSpell(spellId, bot);
            }
        }
    }
    return false;
}

bool RemoveElectrifiedNetAction::isUseful()
{
    return AI_VALUE(bool, "steamrigger electrified net");
}

bool AttackSteamriggerMechanicAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find mechanics
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 50.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 50.0f);

    Unit* mechanic = nullptr;
    float closestDistance = 50.0f;

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_STEAMRIGGER_MECHANIC && AttackersValue::IsValidTarget(unit, bot))
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                mechanic = unit;
                closestDistance = distance;
            }
        }
    }

    if (mechanic)
    {
        return Attack(mechanic);
    }

    return false;
}

bool AttackSteamriggerMechanicAction::isUseful()
{
    return AI_VALUE(bool, "steamrigger mechanic active");
}

// Warlord Kalithresh Actions
bool StopCastingSpellReflectionAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_WARLORD_KALITHRESH, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Stop casting if boss has spell reflection
    if (boss->HasAura(SPELL_SPELL_REFLECTION))
    {
        if (bot->HasUnitState(UNIT_STATE_CASTING))
        {
            bot->InterruptNonMeleeSpells(false);
            return true;
        }
    }

    return false;
}

bool StopCastingSpellReflectionAction::isUseful()
{
    return AI_VALUE(bool, "kalithresh spell reflection");
}

bool HealImpaleTargetAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Prioritize healing impaled targets
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Unit* impaleTarget = nullptr;
    float lowestHealth = 100.0f;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (member->HasAura(SPELL_IMPALE))
        {
            float healthPct = member->GetHealthPct();
            if (healthPct < lowestHealth)
            {
                impaleTarget = member;
                lowestHealth = healthPct;
            }
        }
    }

    if (impaleTarget)
    {
        std::list<uint32> spellIds = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "heal")->Get();
        for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
        {
            uint32 spellId = *it;
            if (botAI->CanCastSpell(spellId, impaleTarget, false))
            {
                return botAI->CastSpell(spellId, impaleTarget);
            }
        }
    }

    return false;
}

bool HealImpaleTargetAction::isUseful()
{
    return AI_VALUE(bool, "kalithresh impale");
}

bool AttackNagaDistillerAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find active Naga Distillers
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 100.0f);

    Unit* distiller = nullptr;
    float closestDistance = 100.0f;

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        // Only attack distillers that are selectable (active)
        if (unit->GetEntry() == NPC_NAGA_DISTILLER && !unit->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        {
            float distance = bot->GetDistance(unit);
            if (distance < closestDistance)
            {
                distiller = unit;
                closestDistance = distance;
            }
        }
    }

    if (distiller)
    {
        return Attack(distiller);
    }

    return false;
}

bool AttackNagaDistillerAction::isUseful()
{
    return AI_VALUE(bool, "kalithresh naga distiller active");
}

bool InterruptDistillerChannelAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Find distiller channeling on Kalithresh
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_NAGA_DISTILLER)
        {
            if (unit->HasUnitState(UNIT_STATE_CASTING) && unit->FindCurrentSpellBySpellId(SPELL_WARLORDS_RAGE_DISTILLER))
            {
                // Interrupt the channel
                std::list<uint32> spellIds = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "interrupt")->Get();
                for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
                {
                    uint32 spellId = *it;
                    if (botAI->CanCastSpell(spellId, unit, false))
                    {
                        return botAI->CastSpell(spellId, unit);
                    }
                }
            }
        }
    }

    return false;
}

bool InterruptDistillerChannelAction::isUseful()
{
    return AI_VALUE(bool, "kalithresh channeling rage");
}