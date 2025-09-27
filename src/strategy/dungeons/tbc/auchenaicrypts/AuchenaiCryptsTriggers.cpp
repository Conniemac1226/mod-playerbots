#include "AuchenaiCryptsTriggers.h"
#include "Playerbots.h"
#include "AttackersValue.h"
#include "strategy/dungeons/tbc/TbcDungeonHelpers.h"

bool ShirrakFocusFireSpawnedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    bool focusWithin10 = false;
    bool focusWithin15 = false;
    TbcDungeon::ForEachNearbyNpc(botAI, bot, 60.0f, [&](Unit* unit)
    {
        if (unit->GetEntry() != NPC_FOCUS_FIRE)
            return;

        float distance = bot->GetDistance(unit);
        if (distance < 10.0f)
            focusWithin10 = true;
        if (distance < 15.0f)
            focusWithin15 = true;
    });

    if (focusWithin10)
        return true;
    
    // FALLBACK: Check if boss is casting Focus Cast (happens after creature spawn)
    Unit* boss = AI_VALUE2(Unit*, "find target", "shirrak the dead watcher");
    if (boss && boss->IsAlive() && boss->IsInCombat())
    {
        if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_FOCUS_CAST))
        {
            // Boss is casting - check for any Focus Fire near us
            if (focusWithin15)
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
    TbcDungeon::ForEachNearbyNpc(botAI, bot, 60.0f, [&](Unit* unit)
    {
        if (unit->GetEntry() == NPC_FOCUS_FIRE)
            focusFireExists = true;
    });
    
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
