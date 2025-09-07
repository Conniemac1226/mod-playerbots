#include "BloodFurnaceActions.h"
#include "Playerbots.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "Value.h"

// The Maker - Avoid Exploding Beaker targeted locations
bool MakerExplodingBeakerAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "the maker");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Exploding Beaker every 7-11s - boss_the_maker.cpp:49-52
    // Targets random player, explodes at location - spread out!
    if (boss->FindCurrentSpellBySpellId(SPELL_EXPLODING_BEAKER))
    {
        // Check who is targeted
        Unit* target = boss->GetVictim();
        if (boss->GetTarget())
        {
            if (Player* targetPlayer = ObjectAccessor::GetPlayer(*boss, boss->GetTarget()))
                target = targetPlayer;
        }
        
        // If we're the target or near the target, move away immediately
        if (target)
        {
            float distToTarget = bot->GetExactDist2d(target);
            if (target == bot || distToTarget < 10.0f)
            {
                // Move away from impact zone quickly
                float angle = frand(0, 2 * M_PI);
                float x = bot->GetPositionX() + cos(angle) * 15.0f;
                float y = bot->GetPositionY() + sin(angle) * 15.0f;
                float z = bot->GetPositionZ();
                return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, 
                            MovementPriority::MOVEMENT_FORCED);
            }
        }
        
        // General spread to avoid splash damage
        Group* group = bot->GetGroup();
        if (group)
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member != bot && member->IsAlive())
                {
                    float dist = bot->GetExactDist2d(member);
                    if (dist < 8.0f)
                    {
                        float angle = bot->GetAngle(member) + M_PI;
                        float x = bot->GetPositionX() + cos(angle) * 10.0f;
                        float y = bot->GetPositionY() + sin(angle) * 10.0f;
                        return MoveTo(bot->GetMapId(), x, y, bot->GetPositionZ());
                    }
                }
            }
        }
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

    return boss->FindCurrentSpellBySpellId(SPELL_EXPLODING_BEAKER);
}

// Handle Domination mind control effect
bool MakerDominationAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "the maker");
    if (!boss || !boss->IsAlive())
        return false;

    // RESEARCHED: Domination every 2 min - boss_the_maker.cpp:53-56
    // Check for dominated allies to help dispel
    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && member->HasAura(SPELL_DOMINATION))
            {
                // Dispel dominated ally if we can
                if (PlayerbotAI::IsHeal(bot) || bot->getClass() == CLASS_PRIEST)
                {
                    if (botAI->CanCastSpell(988, member, false)) // Dispel Magic
                    {
                        return botAI->CastSpell(988, member);
                    }
                }
                
                // If ally is dominated and we can't dispel, spread out
                float dist = bot->GetExactDist2d(member);
                if (dist < 15.0f)
                {
                    float angle = bot->GetAngle(member) + M_PI;
                    float x = bot->GetPositionX() + cos(angle) * 20.0f;
                    float y = bot->GetPositionY() + sin(angle) * 20.0f;
                    return MoveTo(bot->GetMapId(), x, y, bot->GetPositionZ());
                }
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

    // Check if any ally is dominated
    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && member->HasAura(SPELL_DOMINATION))
                return true;
        }
    }

    return false;
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

    // RESEARCHED: Poison Cloud grows over time - boss_broggok.cpp:83-86 & spell script 114-133
    // Cloud starts small and expands, move out IMMEDIATELY
    if (boss->FindCurrentSpellBySpellId(SPELL_POISON_CLOUD) || boss->HasAura(SPELL_POISON_CLOUD))
    {
        // Poison cloud grows to massive size, get far away fast
        float safeDistance = 30.0f;
        float currentDist = bot->GetExactDist2d(boss);
        
        if (currentDist < safeDistance)
        {
            // Move away urgently - cloud is expanding!
            float angle = boss->GetAngle(bot) + M_PI;
            float moveDistance = safeDistance - currentDist + 5.0f;
            float x = boss->GetPositionX() + cos(angle) * (currentDist + moveDistance);
            float y = boss->GetPositionY() + sin(angle) * (currentDist + moveDistance);
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, 
                        MovementPriority::MOVEMENT_FORCED);
        }
    }
    
    // Also check if we're taking poison damage
    if (bot->HasAura(SPELL_POISON))
    {
        // We're in the cloud, get out NOW
        float angle = boss->GetAngle(bot) + M_PI;
        float x = boss->GetPositionX() + cos(angle) * 35.0f;
        float y = boss->GetPositionY() + sin(angle) * 35.0f;
        return MoveTo(bot->GetMapId(), x, y, bot->GetPositionZ(), 
                    false, false, false, true, MovementPriority::MOVEMENT_FORCED);
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

    return boss->FindCurrentSpellBySpellId(SPELL_POISON_CLOUD) || 
           boss->HasAura(SPELL_POISON_CLOUD) || 
           bot->HasAura(SPELL_POISON);
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
        // RESEARCHED: Pattern from HellfireRampartsActions.cpp:138-145 - SAFE PATTERN
        Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "interrupt");
        if (spellIdsValue)
        {
            std::list<uint32> spellIds = spellIdsValue->Get();
            for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
            {
                uint32 spellId = *it;
                if (botAI->CanCastSpell(spellId, boss, false))
                {
                    return botAI->CastSpell(spellId, boss);
                }
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

    // RESEARCHED: Slime Spray frontal cone - boss_broggok.cpp:75-78
    // Cast every 7-12s, move preemptively if in front
    if (boss->FindCurrentSpellBySpellId(SPELL_BROGGOK_SLIME_SPRAY))
    {
        // Check if we're in the frontal cone
        if (boss->HasInArc(M_PI / 3, bot)) // 60 degree frontal cone
        {
            // Move behind boss immediately
            float angle = boss->GetOrientation() + M_PI;
            float x = boss->GetPositionX() + cos(angle) * 8.0f;
            float y = boss->GetPositionY() + sin(angle) * 8.0f;
            float z = boss->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, 
                        MovementPriority::MOVEMENT_FORCED);
        }
    }
    
    // Non-tanks should always stay behind
    if (!botAI->IsTank(bot) && botAI->IsMelee(bot))
    {
        if (boss->HasInArc(M_PI / 2, bot))
        {
            float angle = boss->GetOrientation() + M_PI;
            float x = boss->GetPositionX() + cos(angle) * 5.0f;
            float y = boss->GetPositionY() + sin(angle) * 5.0f;
            return MoveTo(bot->GetMapId(), x, y, boss->GetPositionZ());
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

    // Check if boss is casting OR if we're in front (non-tanks)
    return (boss->FindCurrentSpellBySpellId(SPELL_BROGGOK_SLIME_SPRAY) && boss->HasInArc(M_PI / 3, bot)) ||
           (!botAI->IsTank(bot) && botAI->IsMelee(bot) && boss->HasInArc(M_PI / 2, bot));
}

// Kelidan - Attack Shadowmoon Channelers first
bool AttackShadowmoonChannelerAction::Execute(Event event)
{
    Unit* channeler = AI_VALUE2(Unit*, "find target", "shadowmoon channeler");
    if (!channeler || AI_VALUE(Unit*, "current target") == channeler)
    {
        return false;
    }
    return Attack(channeler);
}

bool AttackShadowmoonChannelerAction::isUseful() 
{ 
    return !botAI->IsHeal(bot); 
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

    return boss->HasAura(SPELL_BURNING_NOVA) && bot->GetExactDist(boss) < 30.0f;
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
        // RESEARCHED: Pattern from HellfireRampartsActions.cpp:138-145 - SAFE PATTERN
        Value<std::list<uint32>>* spellIdsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("spell list", "interrupt");
        if (spellIdsValue)
        {
            std::list<uint32> spellIds = spellIdsValue->Get();
            for (std::list<uint32>::iterator it = spellIds.begin(); it != spellIds.end(); ++it)
            {
                uint32 spellId = *it;
                if (botAI->CanCastSpell(spellId, boss, false))
                {
                    return botAI->CastSpell(spellId, boss);
                }
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

    return boss->FindCurrentSpellBySpellId(SPELL_VORTEX) || boss->HasAura(SPELL_VORTEX);
}