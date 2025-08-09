#include "AuchenaiCryptsTriggers.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"
#include "AttackersValue.h"

bool ShirrakFocusFireSpawnedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // FIRST: Check if Shirrak is CASTING Focus Fire (spell 32300)
    // This gives us earlier warning before the Focus Fire creature spawns
    Unit* boss = AI_VALUE2(Unit*, "find target", "shirrak the dead watcher");
    if (boss && boss->IsAlive())
    {
        // Check if boss is casting Focus Cast (the spell that summons Focus Fire)
        if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_FOCUS_CAST))
        {
            return true; // Start moving immediately when cast begins
        }
    }

    // SECOND: Check for Focus Fire creature (fallback if we missed the cast)
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 60.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 60.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        // Environmental hazard detection
        if (unit->GetEntry() == NPC_FOCUS_FIRE)
        {
            return true;
        }
    }
    
    return false;
}

bool ShirrakAttractMagicTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check for Attract Magic cast
    Unit* boss = AI_VALUE2(Unit*, "find target", "shirrak the dead watcher");
    if (!boss)
        return false;

    // Check if boss is casting Attract Magic
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_ATTRACT_MAGIC);
}

bool ShirrakFocusFireEndedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Per-bot tracking of Focus Fire state changes
    static std::map<ObjectGuid, bool> hadFocusFireMap;
    ObjectGuid botGuid = bot->GetGUID();
    
    // Check if Focus Fire creature currently exists
    bool focusFireExists = false;
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 60.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 60.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_FOCUS_FIRE)
        {
            focusFireExists = true;
            break;
        }
    }
    
    bool hadFocusFire = hadFocusFireMap[botGuid];
    
    // Trigger when Focus Fire existed before but doesn't now
    if (hadFocusFire && !focusFireExists)
    {
        hadFocusFireMap[botGuid] = false;
        return true;
    }
    
    hadFocusFireMap[botGuid] = focusFireExists;
    return false;
}