#include "UnderbogTriggers.h"
#include "Playerbots.h"
#include "UnderbogActions.h"

// Hungarfen - Mushrooms nearby at 20% health
bool HungarfenMushroomNearbyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_HUNGARFEN, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Mushrooms spawn at 20% health - boss_hungarfen.cpp:117
    if (boss->GetHealthPct() > 20.0f)
        return false;

    Unit* mushroom = bot->FindNearestCreature(NPC_UNDERBOG_MUSHROOM, 15.0f);
    return mushroom && mushroom->IsAlive() && bot->GetDistance(mushroom) < 10.0f;
}

// Foul Spores AoE
bool HungarfenFoulSporesTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_HUNGARFEN, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Foul Spores at 20% health - boss_hungarfen.cpp:59
    return boss->GetHealthPct() <= 20.0f && boss->HasAura(UB_SPELL_FOUL_SPORES);
}

// Ghazan - Acid Breath frontal cone
bool GhazanAcidBreathTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_GHAZAN, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Acid Breath frontal check - boss_ghazan.cpp:69
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(UB_SPELL_ACID_BREATH) && 
           boss->HasInArc(M_PI / 3, bot);
}

// Tail Sweep behind boss
bool GhazanTailSweepTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_GHAZAN, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Only trigger if we're behind the boss - boss_ghazan.cpp:77
    return !boss->HasInArc(M_PI, bot);
}

// Swamplord - Windcaller Claw bear pet
bool WindcallerClawActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* bear = bot->FindNearestCreature(NPC_WINDCALLER_CLAW, 100.0f);
    return bear && bear->IsAlive() && bear->IsInCombat();
}

// Freezing Trap being cast
bool MuselekFreezingTrapTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_SWAMPLORD_MUSELEK, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Freezing trap cast check - boss_swamplord_muselek.cpp:138
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(UB_SPELL_THROW_FREEZING_TRAP);
}

// Hunter's Mark debuff
bool MuselekHuntersMarkTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Hunter's Mark debuff check - boss_swamplord_muselek.cpp:155
    return bot->HasAura(UB_SPELL_HUNTERS_MARK);
}

// The Black Stalker - Levitate mechanic
bool BlackStalkerLevitateTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Levitate sequence check - boss_the_black_stalker.cpp:25-35
    return bot->HasAura(UB_SPELL_LEVITATE) || bot->HasAura(SPELL_SUSPENSION);
}

// Spore Striders spawned
bool SporeStriderActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* strider = bot->FindNearestCreature(NPC_SPORE_STRIDER, 50.0f);
    return strider && strider->IsAlive() && strider->IsInCombat();
}

// Chain Lightning being cast
bool BlackStalkerChainLightningTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_BLACK_STALKER, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Chain Lightning cast check - boss_the_black_stalker.cpp:72
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(UB_SPELL_CHAIN_LIGHTNING);
}