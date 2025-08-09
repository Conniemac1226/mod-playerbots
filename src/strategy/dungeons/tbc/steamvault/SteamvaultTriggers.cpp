#include "SteamvaultTriggers.h"
#include "SpellInfo.h"
#include "Unit.h"
#include "Spell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"

// Hydromancer Thespia
bool ThespiaLightningCloudTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_HYDROMANCER_THESPIA, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if boss is casting Lightning Cloud
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_LIGHTNING_CLOUD);
}

bool ThespiaLungBurstTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if bot has Lung Burst debuff
    return bot->HasAura(SPELL_LUNG_BURST);
}

bool ThespiaWaterElementalActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check for water elementals
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 50.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 50.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_THESPIA_WATER_ELEMENTAL && unit->IsInCombat())
            return true;
    }
    return false;
}

// Mekgineer Steamrigger
bool SteamriggerShrinkRayTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if bot has shrink ray debuff
    return bot->HasAura(SPELL_SUPER_SHRINK_RAY);
}

bool SteamriggerSawBladeTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_MEKGINEER_STEAMRIGGER, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if boss is casting Saw Blade
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_SAW_BLADE);
}

bool SteamriggerElectrifiedNetTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if bot has electrified net debuff
    return bot->HasAura(SPELL_ELECTRIFIED_NET);
}

bool SteamriggerMechanicActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Steamrigger is in combat first
    Unit* boss = bot->FindNearestCreature(NPC_MEKGINEER_STEAMRIGGER, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check for mechanics
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 50.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 50.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_STEAMRIGGER_MECHANIC && unit->IsInCombat())
            return true;
    }
    return false;
}

// Warlord Kalithresh
bool KalithreshSpellReflectionTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_WARLORD_KALITHRESH, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if boss has spell reflection buff
    return boss->HasAura(SPELL_SPELL_REFLECTION);
}

bool KalithreshImpaleTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if bot has impale debuff
    return bot->HasAura(SPELL_IMPALE);
}

bool KalithreshNagaDistillerActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Kalithresh is in combat
    Unit* boss = bot->FindNearestCreature(NPC_WARLORD_KALITHRESH, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check for Naga Distillers that are selectable (active)
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_NAGA_DISTILLER && !unit->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            return true;
    }
    return false;
}

bool KalithreshChannelingRageTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check for distiller channeling rage on Kalithresh
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
                return true;
        }
    }
    return false;
}