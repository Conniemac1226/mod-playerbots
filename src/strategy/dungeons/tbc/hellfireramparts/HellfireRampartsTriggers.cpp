#include "HellfireRampartsTriggers.h"
#include "HellfireRampartsActions.h"
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
    if (botAI->IsHeal(bot)) { return false; }

    // Target is not findable from threat table using AI_VALUE2(),
    // therefore need to search manually for the unit name
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");

    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() && unit->GetEntry() == NPC_FIENDISH_HOUND)
        {
            return true;
        }
    }
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

    // RESEARCHED: Treacherous Aura - boss_omor_the_unscarred.cpp
    // Some cores apply heroic variant aura id to players
    return bot->HasAura(SPELL_TREACHEROUS_AURA) || bot->HasAura(SPELL_TREACHEROUS_AURA_H);
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
            
        // RESEARCHED: Liquid Fire summoned by SPELL_SUMMON_LIQUID_FIRE (31706) - HellfireRampartsActions.h:21
        if (unit->GetEntry() == NPC_LIQUID_FIRE)
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

bool OmorTreacheryCastTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Reliable boss detection via NPC entry instead of name lookup
    Unit* boss = bot->FindNearestCreature(NPC_OMOR_THE_UNSCARRED, 100.0f, true);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Detect when Omor STARTS casting Treacherous Aura
    // Spell ID verified in server script (30695); include heroic variant (37566) if used as cast on some cores
    if (!boss->HasUnitState(UNIT_STATE_CASTING))
        return false;
    return boss->FindCurrentSpellBySpellId(SPELL_TREACHEROUS_AURA) || boss->FindCurrentSpellBySpellId(SPELL_TREACHEROUS_AURA_H);
}

bool OmorDebuffAvoidanceTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if any group member has Treacherous Aura debuff
    // Bots need to stay away from debuffed players
    GuidVector friendlyUnits = AI_VALUE(GuidVector, "nearest friendly players");
    for (const auto& guid : friendlyUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit != bot && unit->IsAlive() && (unit->HasAura(SPELL_TREACHEROUS_AURA) || unit->HasAura(SPELL_TREACHEROUS_AURA_H)))
        {
            // Someone has the debuff - check if we're too close
            if (bot->GetDistance(unit) < 15.0f)
                return true;
        }
    }

    return false;
}

bool OmorClearSpreadTrigger::IsActive()
{
    // Trigger when we have spread distance set but should clear it
    if (AI_VALUE(float, "disperse distance") <= 0.0f)
        return false;

    // Use NPC entry for robust boss detection
    Unit* boss = bot->FindNearestCreature(NPC_OMOR_THE_UNSCARRED, 100.0f, true);
    if (!boss)
        return true; // Clear spread when boss is gone

    // Clear spread when Omor finishes casting Treacherous Aura (either id if used for cast)
    if (boss->HasUnitState(UNIT_STATE_CASTING))
    {
        if (boss->FindCurrentSpellBySpellId(SPELL_TREACHEROUS_AURA) || boss->FindCurrentSpellBySpellId(SPELL_TREACHEROUS_AURA_H))
            return false;
    }
    return true;
}
