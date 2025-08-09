#include "MagistersTerraceTriggers.h"
#include "SpellInfo.h"
#include "Unit.h"
#include "Spell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"

// Kael'thas
bool KaelthasCastingPyroblastTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_KAELTHAS, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_PYROBLAST);
}

bool KaelthasCastingGravityLapseTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_KAELTHAS, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_GRAVITY_LAPSE);
}

bool KaelthasArcaneSphereNearbyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 15.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 15.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_ARCANE_SPHERE)
            return true;
    }
    return false;
}

// Vexallus
bool VexallusPureEnergySpawnedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 50.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 50.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_PURE_ENERGY)
            return true;
    }
    return false;
}

// Selin Fireheart
bool SelinFireheartFelExplosionTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_SELIN_FIREHEART, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_FEL_EXPLOSION);
}

bool SelinFireheartChannelingTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_SELIN_FIREHEART, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if boss is channeling Mana Rage on a crystal
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_MANA_RAGE);
}

bool FelCrystalNearbyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Selin is channeling and if there's a crystal nearby
    Unit* boss = bot->FindNearestCreature(NPC_SELIN_FIREHEART, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Only look for crystals when Selin is channeling
    if (!(boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_MANA_RAGE)))
        return false;

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 50.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 50.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_FEL_CRYSTAL && !unit->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            return true;
    }
    return false;
}

// Delrissa
bool DelrissaAddActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Delrissa is engaged
    Unit* boss = bot->FindNearestCreature(NPC_DELRISSA, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check for her adds (various entries)
    const uint32 delrissaAdds[] = {24557, 24558, 24554, 24561, 24559, 24555, 24553, 24556};
    
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 50.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 50.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        for (uint32 addId : delrissaAdds)
        {
            if (unit->GetEntry() == addId && unit->IsInCombat())
                return true;
        }
    }
    return false;
}