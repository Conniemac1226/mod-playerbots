#include "ManaTombsActions.h"
#include "Playerbots.h"

// Pandemonius - Dark Shell reflects damage
bool PandemoniusDarkShellAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_PANDEMONIUS, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Dark Shell reflects damage - boss_pandemonius.cpp:78
    if (boss->HasAura(SPELL_DARK_SHELL))
    {
        // Stop attacking during Dark Shell
        bot->AttackStop();
        
        // Move to max range if melee
        if (botAI->IsMelee(bot))
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

bool PandemoniusDarkShellAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_PANDEMONIUS, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasAura(SPELL_DARK_SHELL);
}

// Spread for Void Blast
bool PandemoniusVoidBlastAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_PANDEMONIUS, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Void Blast random target - boss_pandemonius.cpp:89
    // Spread out to minimize chain damage
    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (member && member != bot && member->IsAlive())
            {
                float distance = bot->GetDistance(member);
                if (distance < 8.0f)
                {
                    // Move away from nearby players
                    float angle = bot->GetAngle(member) + M_PI;
                    float x = bot->GetPositionX() + cos(angle) * 10.0f;
                    float y = bot->GetPositionY() + sin(angle) * 10.0f;
                    float z = bot->GetPositionZ();
                    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
                }
            }
        }
    }

    return false;
}

bool PandemoniusVoidBlastAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_PANDEMONIUS, 50.0f);
    return boss && boss->IsAlive() && boss->IsInCombat();
}

// Tavarok - Earthquake AoE
bool TavarokEarthquakeAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_TAVAROK, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Earthquake AoE - boss_tavarok.cpp:49
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_EARTHQUAKE))
    {
        // Move away from boss during earthquake
        float angle = bot->GetAngle(boss) + M_PI;
        float x = bot->GetPositionX() + cos(angle) * 20.0f;
        float y = bot->GetPositionY() + sin(angle) * 20.0f;
        float z = bot->GetPositionZ();
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool TavarokEarthquakeAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_TAVAROK, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_EARTHQUAKE);
}

// Break Crystal Prison on allies
bool TavarokCrystalPrisonAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Crystal Prison on random target - boss_tavarok.cpp:53
    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (member && member != bot && member->IsAlive())
            {
                if (member->HasAura(SPELL_CRYSTAL_PRISON))
                {
                    // Attack the imprisoned player to break them out
                    return Attack(member);
                }
            }
        }
    }

    return false;
}

bool TavarokCrystalPrisonAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (member && member != bot && member->IsAlive() && member->HasAura(SPELL_CRYSTAL_PRISON))
            return true;
    }

    return false;
}

// Avoid Arcing Smash frontal cleave
bool TavarokArcingSmashAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_TAVAROK, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Arcing Smash frontal - boss_tavarok.cpp:57
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_ARCING_SMASH))
    {
        // Check if we're in front arc
        if (boss->HasInArc(M_PI / 3, bot))
        {
            // Move to side/behind
            float angle = boss->GetOrientation() + (M_PI / 2);
            float x = boss->GetPositionX() + cos(angle) * 10.0f;
            float y = boss->GetPositionY() + sin(angle) * 10.0f;
            float z = boss->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool TavarokArcingSmashAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_TAVAROK, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_ARCING_SMASH) && 
           boss->HasInArc(M_PI / 3, bot);
}

// Nexus-Prince Shaffar - Attack Ethereal Beacons
bool AttackEtherealBeaconAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Ethereal Beacon summons - boss_nexusprince_shaffar.cpp:92
    Unit* beacon = bot->FindNearestCreature(NPC_ETHEREAL_BEACON, 50.0f);
    if (beacon && beacon->IsAlive())
    {
        return Attack(beacon);
    }

    return false;
}

bool AttackEtherealBeaconAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* beacon = bot->FindNearestCreature(NPC_ETHEREAL_BEACON, 50.0f);
    return beacon && beacon->IsAlive();
}

// Avoid Frost Nova
bool ShaffarFrostNovaAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_NEXUSPRINCE_SHAFFAR, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Frost Nova AoE - boss_nexusprince_shaffar.cpp:100
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_FROSTNOVA))
    {
        float distance = bot->GetDistance(boss);
        if (distance < 15.0f)
        {
            // Move away from boss
            float angle = bot->GetAngle(boss) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 20.0f;
            float y = bot->GetPositionY() + sin(angle) * 20.0f;
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool ShaffarFrostNovaAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_NEXUSPRINCE_SHAFFAR, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_FROSTNOVA) && 
           bot->GetDistance(boss) < 15.0f;
}

// Reposition after Shaffar blinks
bool ShaffarBlinkAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_NEXUSPRINCE_SHAFFAR, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Blink teleport - boss_nexusprince_shaffar.cpp:105
    // Re-engage after blink
    return Attack(boss);
}

bool ShaffarBlinkAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_NEXUSPRINCE_SHAFFAR, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if we're out of range after blink
    return bot->GetDistance(boss) > 30.0f;
}

// Yor - Avoid Double Breath frontal cone
bool YorDoubleBreathAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_YOR, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Double Breath frontal - boss_nexusprince_shaffar.cpp:168
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_DOUBLE_BREATH))
    {
        // Check if we're in front arc
        if (boss->HasInArc(M_PI / 3, bot))
        {
            // Move to side/behind
            float angle = boss->GetOrientation() + (M_PI / 2);
            float x = boss->GetPositionX() + cos(angle) * 10.0f;
            float y = boss->GetPositionY() + sin(angle) * 10.0f;
            float z = boss->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool YorDoubleBreathAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_YOR, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_DOUBLE_BREATH) && 
           boss->HasInArc(M_PI / 3, bot);
}

// Avoid Stomp AoE
bool YorStompAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_YOR, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Stomp AoE - boss_nexusprince_shaffar.cpp:173
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_STOMP))
    {
        float distance = bot->GetDistance(boss);
        if (distance < 15.0f)
        {
            // Move away from boss
            float angle = bot->GetAngle(boss) + M_PI;
            float x = bot->GetPositionX() + cos(angle) * 20.0f;
            float y = bot->GetPositionY() + sin(angle) * 20.0f;
            float z = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool YorStompAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_YOR, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_STOMP) && 
           bot->GetDistance(boss) < 15.0f;
}