#include "BloodFurnaceTriggers.h"
#include "BloodFurnaceActions.h"
#include "Playerbots.h"
#include "Value.h"

// The Maker - Exploding Beaker trigger
bool TheMakerExplodingBeakerTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_THE_MAKER, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Exploding Beaker spell from boss_the_maker.cpp:51
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_EXPLODING_BEAKER);
}

// The Maker - Domination trigger
bool TheMakerDominationTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Domination effect from boss_the_maker.cpp:55
    return bot->HasAura(SPELL_DOMINATION);
}

// Broggok - Poison Cloud trigger
bool BroggokPoisonCloudTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_BROGGOK, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Poison Cloud spell from boss_broggok.cpp:85
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_POISON_CLOUD);
}

// Broggok - Interrupt Poison Bolt trigger
bool BroggokInterruptPoisonBoltTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_BROGGOK, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Poison Bolt spell from boss_broggok.cpp:81
    // Only trigger if bot can interrupt
    if (!boss->HasUnitState(UNIT_STATE_CASTING) || !boss->FindCurrentSpellBySpellId(SPELL_POISON_BOLT))
        return false;

    // Check if bot has interrupt spells available - SAFE PATTERN
    Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "interrupt");
    if (spellIdsValue)
    {
        std::list<uint32> spellIds = spellIdsValue->Get();
        for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
        {
            uint32 spellId = *it;
            if (botAI->CanCastSpell(spellId, boss, false))
            {
                return true;
            }
        }
    }

    return false;
}

// Broggok - Slime Spray trigger
bool BroggokSlimeSprayTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_BROGGOK, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Slime Spray spell from boss_broggok.cpp:77
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_BROGGOK_SLIME_SPRAY))
    {
        // Check if we're in front arc of the boss
        return boss->HasInArc(M_PI / 2, bot);
    }

    return false;
}

// Kelidan - Shadowmoon Channeler priority target trigger
bool KelidanShadowmoonChannelerTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Kelidan encounter is active
    Unit* boss = bot->FindNearestCreature(NPC_KELIDAN, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Channelers must be killed first - boss_kelidan_the_breaker.cpp:113
    Unit* channeler = bot->FindNearestCreature(NPC_SHADOWMOON_CHANNELER, 100.0f);
    if (channeler && channeler->IsAlive())
    {
        // Check if current target is not a channeler
        Unit* currentTarget = AI_VALUE(Unit*, "current target");
        return !currentTarget || currentTarget->GetEntry() != NPC_SHADOWMOON_CHANNELER;
    }

    return false;
}

// Kelidan - Burning Nova trigger
bool KelidanBurningNovaTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_KELIDAN, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Burning Nova aura from boss_kelidan_the_breaker.cpp:90
    if (boss->HasAura(SPELL_BURNING_NOVA))
    {
        // Only trigger if we're too close
        return bot->GetDistance(boss) < 20.0f;
    }

    return false;
}

// Kelidan - Interrupt Shadow Bolt Volley trigger
bool KelidanInterruptShadowBoltVolleyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_KELIDAN, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Shadow Bolt Volley spell from boss_kelidan_the_breaker.cpp:80
    // Only trigger if bot can interrupt
    if (!boss->HasUnitState(UNIT_STATE_CASTING) || !boss->FindCurrentSpellBySpellId(SPELL_SHADOW_BOLT_VOLLEY))
        return false;

    // Check if bot has interrupt spells available - SAFE PATTERN
    Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "interrupt");
    if (spellIdsValue)
    {
        std::list<uint32> spellIds = spellIdsValue->Get();
        for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
        {
            uint32 spellId = *it;
            if (botAI->CanCastSpell(spellId, boss, false))
            {
                return true;
            }
        }
    }

    return false;
}

// Kelidan - Vortex trigger (Heroic only)
bool KelidanVortexTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Vortex only in heroic difficulty - boss_kelidan_the_breaker.cpp:92
    if (!bot->GetMap()->IsHeroic())
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_KELIDAN, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Vortex spell from boss_kelidan_the_breaker.cpp:93
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_VORTEX);
}