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

    // IMMEDIATE: Check for Focus Fire creature spawn (happens 3 seconds before damage)
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 60.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 60.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        // Focus Fire creature detection - spawns 3 seconds before damage
        if (unit->GetEntry() == NPC_FOCUS_FIRE)
        {
            // Check if Focus Fire is near us (within 10 yards means we're the target)
            float distance = bot->GetDistance(unit);
            if (distance < 10.0f)  // Focus Fire spawned on us - MOVE NOW!
                return true;
        }
    }
    
    // FALLBACK: Check if boss is casting Focus Cast (happens after creature spawn)
    Unit* boss = AI_VALUE2(Unit*, "find target", "shirrak the dead watcher");
    if (boss && boss->IsAlive() && boss->IsInCombat())
    {
        if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_FOCUS_CAST))
        {
            // Boss is casting - check for any Focus Fire near us
            for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
            {
                Unit* unit = *i;
                if (unit && unit->GetEntry() == NPC_FOCUS_FIRE && bot->GetDistance(unit) < 15.0f)
                    return true;
            }
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
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
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
    Cell::VisitObjects(bot, searcher, 60.0f);

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