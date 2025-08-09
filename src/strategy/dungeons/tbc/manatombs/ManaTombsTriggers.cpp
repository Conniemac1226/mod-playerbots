#include "ManaTombsTriggers.h"
#include "Playerbots.h"
#include "ManaTombsActions.h"

// Pandemonius - Dark Shell active
bool PandemoniusDarkShellTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_PANDEMONIUS, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Dark Shell buff check - boss_pandemonius.cpp:78
    return boss->HasAura(SPELL_DARK_SHELL);
}

// Void Blast spread check
bool PandemoniusVoidBlastTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_PANDEMONIUS, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if players are too close together
    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (member && member != bot && member->IsAlive())
            {
                if (bot->GetDistance(member) < 8.0f)
                    return true;
            }
        }
    }

    return false;
}

// Tavarok - Earthquake casting
bool TavarokEarthquakeTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_TAVAROK, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Earthquake cast check - boss_tavarok.cpp:49
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_EARTHQUAKE);
}

// Crystal Prison on allies
bool TavarokCrystalPrisonTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Crystal Prison debuff check - boss_tavarok.cpp:53
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

// Arcing Smash frontal check
bool TavarokArcingSmashTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_TAVAROK, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Arcing Smash frontal check - boss_tavarok.cpp:57
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_ARCING_SMASH) && 
           boss->HasInArc(M_PI / 3, bot);
}

// Ethereal Beacon spawned
bool EtherealBeaconActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // RESEARCHED: Ethereal Beacon spawn check - boss_nexusprince_shaffar.cpp:92
    Unit* beacon = bot->FindNearestCreature(NPC_ETHEREAL_BEACON, 50.0f);
    return beacon && beacon->IsAlive();
}

// Frost Nova danger zone
bool ShaffarFrostNovaTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_NEXUSPRINCE_SHAFFAR, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Frost Nova range check - boss_nexusprince_shaffar.cpp:100
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_FROSTNOVA) && 
           bot->GetDistance(boss) < 15.0f;
}

// Shaffar blinked away
bool ShaffarBlinkTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_NEXUSPRINCE_SHAFFAR, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Blink repositioning check - boss_nexusprince_shaffar.cpp:105
    return bot->GetDistance(boss) > 30.0f;
}

// Yor - Double Breath frontal
bool YorDoubleBreathTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_YOR, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Double Breath frontal check - boss_nexusprince_shaffar.cpp:168
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_DOUBLE_BREATH) && 
           boss->HasInArc(M_PI / 3, bot);
}

// Stomp AoE danger
bool YorStompTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_YOR, 50.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // RESEARCHED: Stomp range check - boss_nexusprince_shaffar.cpp:173
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_STOMP) && 
           bot->GetDistance(boss) < 15.0f;
}