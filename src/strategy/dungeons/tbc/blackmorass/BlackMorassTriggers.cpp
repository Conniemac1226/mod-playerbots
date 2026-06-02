#include "BlackMorassTriggers.h"
#include "AiObjectContext.h"
#include "Creature.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Unit.h"

namespace
{
bool IsBossCasting(Unit* boss, uint32 spellId)
{
    return boss && boss->IsAlive() && boss->IsInCombat() && boss->HasUnitState(UNIT_STATE_CASTING) &&
           boss->FindCurrentSpellBySpellId(spellId);
}
}

bool PortalAddActiveTrigger::IsActive()
{
    GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (ObjectGuid const& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->IsAlive() && IsBlackMorassPortalAdd(unit->GetEntry()))
            return true;
    }

    return false;
}

bool MedivhNeedsProtectionTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* medivh = AI_VALUE2(Unit*, "find target", "medivh");
    if (!medivh || !medivh->IsAlive())
        return false;

    if (bot->GetDistance(medivh) > 40.0f)
        return false;

    GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (ObjectGuid const& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->IsAlive() && IsBlackMorassPortalAdd(unit->GetEntry()) && unit->GetDistance(medivh) < 30.0f)
            return true;
    }

    return medivh->GetHealthPct() < 95.0f;
}

bool AeonusCleaveNearbyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot || botAI->IsTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "aeonus");
    return boss && boss->IsAlive() && boss->IsInCombat() && bot->GetDistance(boss) < 10.0f &&
           boss->HasInArc(M_PI / 2, bot);
}

bool AeonusEngagedTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "aeonus");
    return boss && boss->IsAlive() && boss->IsInCombat();
}

bool ChronoLordDejaEngagedTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "chrono lord deja");
    return boss && boss->IsAlive() && boss->IsInCombat();
}

bool TemporusEngagedTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "temporus");
    return boss && boss->IsAlive() && boss->IsInCombat();
}

bool SandBreathDangerTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    Unit* boss = AI_VALUE2(Unit*, "find target", "aeonus");
    return bot && IsBossCasting(boss, SPELL_SAND_BREATH) && bot->GetDistance(boss) < 20.0f &&
           boss->HasInArc(M_PI / 4, bot);
}

bool TimeLapseDangerTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    Unit* boss = AI_VALUE2(Unit*, "find target", "chrono lord deja");
    if (!bot || !IsBossCasting(boss, SPELL_TIME_LAPSE))
        return false;

    GuidVector const members = AI_VALUE(GuidVector, "group members");
    for (ObjectGuid const& member : members)
    {
        Unit* ally = botAI->GetUnit(member);
        if (ally && ally != bot && ally->IsAlive() && bot->GetDistance(ally) < 10.0f)
            return true;
    }

    return false;
}

bool ArcaneDischargeDangerTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    Unit* boss = AI_VALUE2(Unit*, "find target", "chrono lord deja");
    return bot && IsBossCasting(boss, SPELL_ARCANE_DISCHARGE) && bot->GetDistance(boss) < 20.0f;
}

bool AttractionActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    return bot && bot->HasAura(SPELL_ATTRACTION);
}

bool WingBuffetDangerTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    Unit* boss = AI_VALUE2(Unit*, "find target", "temporus");
    return bot && IsBossCasting(boss, SPELL_WING_BUFFET) && bot->GetDistance(boss) < 10.0f;
}

bool MortalWoundActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI->IsHeal(bot))
        return false;

    GuidVector const members = AI_VALUE(GuidVector, "group members");
    for (ObjectGuid const& member : members)
    {
        Unit* ally = botAI->GetUnit(member);
        if (ally && ally->IsAlive() && ally->HasAura(SPELL_MORTAL_WOUND))
            return true;
    }

    return false;
}

bool TemporusReflectActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    Unit* boss = AI_VALUE2(Unit*, "find target", "temporus");
    return bot && boss && boss->IsAlive() && boss->IsInCombat() && boss->HasAura(SPELL_REFLECT) &&
           bot->IsNonMeleeSpellCast(false);
}

bool TemporusHastenActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    if (bot->getClass() != CLASS_PRIEST && bot->getClass() != CLASS_SHAMAN && bot->getClass() != CLASS_WARLOCK)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "temporus");
    return boss && boss->IsAlive() && boss->IsInCombat() && boss->HasAura(SPELL_HASTEN);
}

bool DejaArcaneBlastCastingTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    switch (bot->getClass())
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
    return IsBossCasting(boss, SPELL_ARCANE_BLAST) && bot->GetDistance(boss) <= 30.0f;
}
