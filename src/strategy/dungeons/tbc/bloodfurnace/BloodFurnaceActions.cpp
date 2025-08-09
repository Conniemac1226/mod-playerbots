#include "BloodFurnaceActions.h"
#include "Playerbots.h"
#include "Group.h"

// The Maker - Avoid Exploding Beaker targeted locations
bool MakerExplodingBeakerAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "the maker");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Exploding Beaker from boss_the_maker.cpp:51
    // Move away from targeted location when beaker is incoming
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_EXPLODING_BEAKER))
    {
        // Move to a safe distance from the targeted area
        float angle = bot->GetAngle(boss) + M_PI / 4;  // Move at an angle
        float x = bot->GetPositionX() + cos(angle) * 8.0f;
        float y = bot->GetPositionY() + sin(angle) * 8.0f;
        float z = bot->GetPositionZ();
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool MakerExplodingBeakerAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "the maker");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_EXPLODING_BEAKER);
}

// Handle Domination mind control effect
bool MakerDominationAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Domination from boss_the_maker.cpp:55 (2 minute cooldown)
    if (bot->HasAura(SPELL_DOMINATION))
    {
        // Try to dispel the domination effect
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

bool MakerDominationAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    return bot->HasAura(SPELL_DOMINATION);
}

// Broggok - Avoid Poison Cloud area effect
bool BroggokAvoidPoisonCloudAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "broggok");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Poison Cloud from boss_broggok.cpp:85 (20 second cooldown)
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_POISON_CLOUD))
    {
        // Move away from boss to avoid poison cloud
        float distance = bot->GetDistance(boss);
        if (distance < 15.0f)  // Move to safe distance
        {
            return FleePosition(boss->GetPosition(), 20.0f, 500U);
        }
    }

    return false;
}

bool BroggokAvoidPoisonCloudAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "broggok");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_POISON_CLOUD);
}

// Interrupt Broggok's Poison Bolt
bool BroggokInterruptPoisonBoltAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "broggok");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Poison Bolt from boss_broggok.cpp:81 (6-11 second cooldown)
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_POISON_BOLT))
    {
        // RESEARCHED: Pattern from HellfireRampartsActions.cpp:138-145
        std::list<uint32> spellIds = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "interrupt")->Get();
        for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
        {
            uint32 spellId = *it;
            if (botAI->CanCastSpell(spellId, boss, false))
            {
                return botAI->CastSpell(spellId, boss);
            }
        }
    }

    return false;
}

bool BroggokInterruptPoisonBoltAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "broggok");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_POISON_BOLT);
}

// Avoid Slime Spray frontal cone
bool BroggokAvoidSlimeSprayAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "broggok");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Slime Spray from boss_broggok.cpp:77 (7-12 second cooldown)
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_BROGGOK_SLIME_SPRAY))
    {
        // Check if we're in front of the boss - move to side or behind
        if (boss->HasInArc(M_PI / 2, bot))
        {
            // Move to boss's side
            float angle = boss->GetOrientation() + M_PI / 2;  // 90 degrees to side
            float x = boss->GetPositionX() + cos(angle) * 8.0f;
            float y = boss->GetPositionY() + sin(angle) * 8.0f;
            float z = boss->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool BroggokAvoidSlimeSprayAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "broggok");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if boss is casting Slime Spray and we're in the front arc
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_BROGGOK_SLIME_SPRAY))
    {
        return boss->HasInArc(M_PI / 2, bot);
    }

    return false;
}

// Kelidan - Attack Shadowmoon Channelers first
bool AttackShadowmoonChannelerAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Channelers must die first - boss_kelidan_the_breaker.cpp:113
    Unit* channeler = AI_VALUE2(Unit*, "find target", "shadowmoon channeler");
    if (channeler && channeler->IsAlive())
    {
        return Attack(channeler);
    }

    return false;
}

bool AttackShadowmoonChannelerAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Kelidan encounter is active and channelers exist
    Unit* boss = AI_VALUE2(Unit*, "find target", "kelidan the breaker");
    if (!boss || !boss->IsInCombat())
        return false;

    Unit* channeler = AI_VALUE2(Unit*, "find target", "shadowmoon channeler");
    if (channeler && channeler->IsAlive())
    {
        // Current target should not be a channeler already
        Unit* currentTarget = AI_VALUE(Unit*, "current target");
        return !currentTarget || currentTarget->GetEntry() != NPC_SHADOWMOON_CHANNELER;
    }

    return false;
}

// Avoid Burning Nova area damage
bool KelidanBurningNovaAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "kelidan the breaker");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Burning Nova from boss_kelidan_the_breaker.cpp:88-99 (25-32 second cooldown)
    if (boss->HasAura(SPELL_BURNING_NOVA))
    {
        // Move away from boss during burning nova
        float distance = bot->GetDistance(boss);
        if (distance < 20.0f)  // Move to safe distance
        {
            float angle = bot->GetAngle(boss) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 25.0f;
            float y = bot->GetPositionY() + sin(angle) * 25.0f;
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool KelidanBurningNovaAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "kelidan the breaker");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasAura(SPELL_BURNING_NOVA) && bot->GetDistance(boss) < 20.0f;
}

// Interrupt Shadow Bolt Volley
bool KelidanInterruptShadowBoltVolleyAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "kelidan the breaker");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Shadow Bolt Volley from boss_kelidan_the_breaker.cpp:80 (8-13 second cooldown)
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_SHADOW_BOLT_VOLLEY))
    {
        // RESEARCHED: Pattern from HellfireRampartsActions.cpp:138-145
        std::list<uint32> spellIds = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "interrupt")->Get();
        for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
        {
            uint32 spellId = *it;
            if (botAI->CanCastSpell(spellId, boss, false))
            {
                return botAI->CastSpell(spellId, boss);
            }
        }
    }

    return false;
}

bool KelidanInterruptShadowBoltVolleyAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "kelidan the breaker");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_SHADOW_BOLT_VOLLEY);
}

// Avoid Vortex (Heroic only)
bool KelidanAvoidVortexAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "kelidan the breaker");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Vortex from boss_kelidan_the_breaker.cpp:93 (Heroic only)
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_VORTEX))
    {
        // Move away from vortex effect
        float distance = bot->GetDistance(boss);
        if (distance < 15.0f)
        {
            float angle = bot->GetAngle(boss) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 20.0f;
            float y = bot->GetPositionY() + sin(angle) * 20.0f;
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool KelidanAvoidVortexAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "kelidan the breaker");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Only in heroic difficulty
    if (!bot->GetMap()->IsHeroic())
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_VORTEX);
}