#include "HellfireRampartsTriggers.h"
#include "Playerbots.h"

// Watchkeeper Gargolmar - Hellfire Watchers join at 50%
bool HellfireWatcherActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Pattern from CharmingTotemSpawnedTrigger in SethekkHallsTriggers.cpp:8-47
    Unit* boss = bot->FindNearestCreature(NPC_WATCHKEEPER_GARGOLMAR, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Watchers join at 50% health - boss_watchkeeper_gargolmar.cpp:60
    if (boss->GetHealthPct() > 50.0f)
        return false;

    Unit* watcher = bot->FindNearestCreature(NPC_HELLFIRE_WATCHER, 100.0f);
    return watcher && watcher->IsAlive() && watcher->IsInCombat();

    return false;
}

// Gargolmar Retaliation at 20% health
bool GargolmarRetaliationTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_WATCHKEEPER_GARGOLMAR, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Retaliation at 20% health - boss_watchkeeper_gargolmar.cpp:70-77
    return boss->HasAura(SPELL_RETALIATION);
}

// Gargolmar Surge casting
bool GargolmarSurgeTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_WATCHKEEPER_GARGOLMAR, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Surge cast every 11s - boss_watchkeeper_gargolmar.cpp:88-95
    // Check if we're far enough to be potential target
    return bot->GetDistance(boss) > 20.0f;
}

// Omor - Fiendish Hounds summoned
bool FiendishHoundActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Fiendish Hound summon - boss_omor_the_unscarred.cpp:78-85
    Unit* hound = bot->FindNearestCreature(NPC_FIENDISH_HOUND, 50.0f);
    return hound && hound->IsAlive() && hound->IsInCombat();

    return false;
}

// Omor casting Shadow Bolt
bool OmorShadowBoltCastTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_OMOR_THE_UNSCARRED, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Shadow Bolt cast - boss_omor_the_unscarred.cpp:134
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_SHADOW_BOLT);
}

// Omor Treacherous Aura on players
bool OmorTreacherousAuraTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Treacherous Aura - boss_omor_the_unscarred.cpp:76
    return bot->HasAura(SPELL_TREACHEROUS_AURA);
}

// Omor Demonic Shield at 21% health
bool OmorDemonicShieldTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_OMOR_THE_UNSCARRED, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Demonic Shield - boss_omor_the_unscarred.cpp:56-62
    return boss->HasAura(SPELL_DEMONIC_SHIELD);
}

// Omor is engaged - positioning check for non-tanks
bool OmorEngagedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_OMOR_THE_UNSCARRED, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Omor doesn't move - boss_omor_the_unscarred.cpp:44
    // Non-tank melees should stay at range
    return !botAI->IsRanged(bot) && !botAI->IsTank(bot) && bot->GetDistance(boss) < 8.0f;
}

// Liquid Fire patches nearby
bool LiquidFireNearbyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Liquid Fire is ground effect, check for fire visuals
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
            
        // Check for fire visual NPCs
        if (unit->GetEntry() == 17084 || unit->GetEntry() == 18240)
        {
            if (bot->GetDistance(unit) < 8.0f)
                return true;
        }
    }

    return false;
}

// Nazan casting Cone of Fire
bool NazanConeOfFireTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* nazan = bot->FindNearestCreature(NPC_NAZAN, 100.0f);
    if (!nazan || !nazan->IsAlive() || !nazan->IsInCombat())
        return false;

    // RESEARCHED: Cone of Fire spell - boss_vazruden_the_herald.cpp:43
    if (nazan->HasUnitState(UNIT_STATE_CASTING) && nazan->FindCurrentSpellBySpellId(SPELL_CONE_OF_FIRE))
    {
        // Check if we're in front arc
        return nazan->HasInArc(M_PI / 2, bot);
    }

    return false;
}

// Nazan has landed (phase 2)
bool NazanLandedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* nazan = bot->FindNearestCreature(NPC_NAZAN, 100.0f);
    Unit* vazruden = bot->FindNearestCreature(NPC_VAZRUDEN, 100.0f);

    // RESEARCHED: Nazan lands when flying phase ends - boss_vazruden_the_herald.cpp:178-186
    if (nazan && nazan->IsAlive() && !nazan->IsLevitating() &&
        vazruden && vazruden->IsAlive())
    {
        // Check if current target is not Nazan
        Unit* currentTarget = AI_VALUE(Unit*, "current target");
        return currentTarget && currentTarget->GetEntry() != NPC_NAZAN;
    }

    return false;
}

// Vazruden alone after Nazan dies
bool VazrudenAloneTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* nazan = bot->FindNearestCreature(NPC_NAZAN, 100.0f);
    Unit* vazruden = bot->FindNearestCreature(NPC_VAZRUDEN, 100.0f);

    // Vazruden alone after Nazan dies
    return (!nazan || !nazan->IsAlive()) && vazruden && vazruden->IsAlive();
}

// Nazan Bellowing Roar (Heroic only)
bool NazanBellowingRoarTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Only in heroic
    if (bot->GetMap()->GetDifficulty() != DUNGEON_DIFFICULTY_HEROIC)
        return false;

    // RESEARCHED: Bellowing Roar fear - boss_vazruden_the_herald.cpp:218-225  
    return bot->HasAura(SPELL_BELLOWING_ROAR);
}