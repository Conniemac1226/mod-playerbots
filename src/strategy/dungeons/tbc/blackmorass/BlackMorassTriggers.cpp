#include "BlackMorassTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Player.h"
#include "Unit.h"
#include "Creature.h"
#include "Value.h"
#include "AiObjectContext.h"

bool AeonusCleaveNearbyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    // Only trigger for non-tanks
    if (botAI->IsTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "aeonus");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Pattern from Karazhan: check if in front arc and close (cleave danger)
    float distance = bot->GetDistance(boss);
    return distance < 10.0f && boss->HasInArc(M_PI / 2, bot);
}

bool AeonusEngagedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "aeonus");
    return boss && boss->IsAlive() && boss->IsInCombat();
}

bool ChronoLordDejaEngagedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "chrono lord deja");
    return boss && boss->IsAlive() && boss->IsInCombat();
}

bool TemporusEngagedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "temporus");
    return boss && boss->IsAlive() && boss->IsInCombat();
}

bool TimeLapseNearbyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "chrono lord deja");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if bot is in Time Lapse AoE range
    float distance = bot->GetDistance(boss);
    return distance < 15.0f;
}

bool ArcaneDischargeNearbyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "chrono lord deja");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if bot is in Arcane Discharge AoE range
    float distance = bot->GetDistance(boss);
    return distance < 20.0f;
}

bool WingBuffetNearbyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "temporus");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    float distance = bot->GetDistance(boss);
    // Trigger when positioning is suboptimal for wing buffet
    return distance > 5.0f && distance < 15.0f;
}

// RESEARCHED: boss_temporus.cpp:54 - HASTEN buff needs dispel
bool TemporusHastenActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    // Only trigger for classes that can dispel
    if (bot->getClass() != CLASS_PRIEST && 
        bot->getClass() != CLASS_SHAMAN &&
        bot->getClass() != CLASS_WARLOCK)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "temporus");
    return boss && boss->IsAlive() && boss->IsInCombat() && boss->HasAura(SPELL_HASTEN);
}

// RESEARCHED: boss_chrono_lord_deja.cpp:58 - ARCANE_BLAST needs interrupt  
bool DejaArcaneBlastCastingTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    // Only trigger for classes with interrupts
    switch(bot->getClass())
    {
        case CLASS_WARRIOR:
        case CLASS_ROGUE:
        case CLASS_SHAMAN:
        case CLASS_MAGE:
        case CLASS_PRIEST:
            break;
        default:
            return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "chrono lord deja");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if boss is within interrupt range and casting Arcane Blast
    float distance = bot->GetDistance(boss);
    return distance <= 30.0f && boss->HasUnitState(UNIT_STATE_CASTING) && 
           boss->FindCurrentSpellBySpellId(SPELL_ARCANE_BLAST);
}

// RESEARCHED: boss_aeonus.cpp:86 - ENRAGE increases damage
bool AeonusEnragedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "aeonus");
    return boss && boss->IsAlive() && boss->IsInCombat() && boss->HasAura(SPELL_ENRAGE);
}