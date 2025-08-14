#ifndef _PLAYERBOT_BLACKMORASSVALUES_H
#define _PLAYERBOT_BLACKMORASSVALUES_H

#include "Value.h"
#include "AttackersValue.h"
#include "PlayerbotAI.h"

// Black Morass NPC IDs and Spell IDs based on research from CLAUDE.md
const uint32 NPC_RIFT_LORD = 17839;
const uint32 NPC_RIFT_KEEPER = 21104;
const uint32 NPC_CHRONO_LORD_DEJA = 17879;
const uint32 NPC_TEMPORUS = 17880;
const uint32 NPC_AEONUS = 17881;
const uint32 NPC_MEDIVH = 15608;

const uint32 SPELL_SAND_BREATH = 31478;
const uint32 SPELL_TIME_STOP = 31422;
const uint32 SPELL_FRENZY = 31540;
const uint32 SPELL_TIME_LAPSE = 31467;
const uint32 SPELL_ARCANE_DISCHARGE = 31472;
const uint32 SPELL_ATTRACTION = 38540;
const uint32 SPELL_WING_BUFFET = 31475;
const uint32 SPELL_MORTAL_WOUND = 25646;
const uint32 SPELL_SPELL_REFLECTION = 31496;

// Portal/Add Management Value
class PortalAddActiveValue : public BoolCalculatedValue
{
public:
    PortalAddActiveValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "portal add active") {}

    bool Calculate() override
    {
        Player* bot = botAI->GetBot();
        if (!bot)
            return false;

        // RESEARCHED: Priority targeting for Black Morass portal adds
        // Rift Lords > Rift Keepers > Other portal adds
        const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        for (auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (!unit || !unit->IsAlive())
                continue;

            // Priority 1: Rift Lords (highest threat)
            if (unit->GetEntry() == NPC_RIFT_LORD)
            {
                float distance = bot->GetExactDist2d(unit);
                if (distance < 50.0f)
                    return true;
            }

            // Priority 2: Rift Keepers 
            if (unit->GetEntry() == NPC_RIFT_KEEPER)
            {
                float distance = bot->GetExactDist2d(unit);
                if (distance < 50.0f)
                    return true;
            }
        }

        return false;
    }
};

// Medivh Protection Value
class MedivhNeedsProtectionValue : public BoolCalculatedValue
{
public:
    MedivhNeedsProtectionValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "medivh needs protection") {}

    bool Calculate() override
    {
        Player* bot = botAI->GetBot();
        if (!bot)
            return false;

        // Find Medivh and check if he's under attack
        const GuidVector npcs = AI_VALUE(GuidVector, "nearest friendly npcs");
        for (auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->GetEntry() == NPC_MEDIVH)
            {
                // Check if Medivh is in combat or being threatened
                if (unit->IsInCombat() || unit->GetHealthPct() < 95.0f)
                {
                    float distance = bot->GetExactDist2d(unit);
                    if (distance < 40.0f)
                        return true;
                }
                break;
            }
        }

        return false;
    }
};

// Aeonus Cleave Danger Value
class AeonusCleaveDangerValue : public BoolCalculatedValue
{
public:
    AeonusCleaveDangerValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "aeonus cleave danger") {}

    bool Calculate() override
    {
        Unit* boss = AI_VALUE2(Unit*, "find target", "aeonus");
        if (!boss || !boss->IsAlive() || !boss->IsInCombat())
            return false;

        Player* bot = botAI->GetBot();
        if (!bot)
            return false;

        float distance = bot->GetExactDist2d(boss);
        // Cleave affects frontal cone, stay at sides/behind
        return distance < 10.0f && bot->isInFront(boss);
    }
};

// Aeonus Engaged Value
class AeonusEngagedValue : public BoolCalculatedValue
{
public:
    AeonusEngagedValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "aeonus engaged") {}

    bool Calculate() override
    {
        Unit* boss = AI_VALUE2(Unit*, "find target", "aeonus");
        return boss && boss->IsAlive() && boss->IsInCombat();
    }
};

// Sand Breath Danger Value
class SandBreathDangerValue : public BoolCalculatedValue
{
public:
    SandBreathDangerValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "sand breath danger") {}

    bool Calculate() override
    {
        Unit* boss = AI_VALUE2(Unit*, "find target", "aeonus");
        if (!boss || !boss->IsAlive() || !boss->IsInCombat())
            return false;

        Player* bot = botAI->GetBot();
        if (!bot)
            return false;

        // Check if Aeonus is casting Sand Breath (frontal cone)
        if (boss->FindCurrentSpellBySpellId(SPELL_SAND_BREATH))
        {
            float distance = bot->GetExactDist2d(boss);
            return distance < 15.0f && bot->isInFront(boss);
        }

        return false;
    }
};

// Time Stop Active Value
class TimeStopActiveValue : public BoolCalculatedValue
{
public:
    TimeStopActiveValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "time stop active") {}

    bool Calculate() override
    {
        Unit* boss = AI_VALUE2(Unit*, "find target", "aeonus");
        if (!boss || !boss->IsAlive() || !boss->IsInCombat())
            return false;

        return boss->HasAura(SPELL_TIME_STOP) || boss->FindCurrentSpellBySpellId(SPELL_TIME_STOP);
    }
};

// Time Lapse Danger Value
class TimeLapseDangerValue : public BoolCalculatedValue
{
public:
    TimeLapseDangerValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "time lapse danger") {}

    bool Calculate() override
    {
        Unit* boss = AI_VALUE2(Unit*, "find target", "chrono lord deja");
        if (!boss || !boss->IsAlive() || !boss->IsInCombat())
            return false;

        Player* bot = botAI->GetBot();
        if (!bot)
            return false;

        // Time Lapse spreads damage among close players
        if (boss->FindCurrentSpellBySpellId(SPELL_TIME_LAPSE))
        {
            const GuidVector members = AI_VALUE(GuidVector, "group members");
            int nearbyCount = 0;
            for (auto& member : members)
            {
                Unit* unit = botAI->GetUnit(member);
                if (!unit)
                    continue;

                float distance = unit->GetExactDist2d(boss);
                if (distance < 10.0f)
                    nearbyCount++;
            }

            // Spread if too few players nearby
            return nearbyCount < 2;
        }

        return false;
    }
};

// Arcane Discharge Danger Value
class ArcaneDischargeValue : public BoolCalculatedValue
{
public:
    ArcaneDischargeValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "arcane discharge danger") {}

    bool Calculate() override
    {
        Unit* boss = AI_VALUE2(Unit*, "find target", "chrono lord deja");
        if (!boss || !boss->IsAlive() || !boss->IsInCombat())
            return false;

        Player* bot = botAI->GetBot();
        if (!bot)
            return false;

        // Move away from Arcane Discharge AoE
        if (boss->FindCurrentSpellBySpellId(SPELL_ARCANE_DISCHARGE))
        {
            float distance = bot->GetExactDist2d(boss);
            return distance < 12.0f;
        }

        return false;
    }
};

// Attraction Active Value
class AttractionActiveValue : public BoolCalculatedValue
{
public:
    AttractionActiveValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "attraction active") {}

    bool Calculate() override
    {
        Unit* boss = AI_VALUE2(Unit*, "find target", "chrono lord deja");
        if (!boss || !boss->IsAlive() || !boss->IsInCombat())
            return false;

        return boss->HasAura(SPELL_ATTRACTION) || boss->FindCurrentSpellBySpellId(SPELL_ATTRACTION);
    }
};

// Wing Buffet Danger Value
class WingBuffetDangerValue : public BoolCalculatedValue
{
public:
    WingBuffetDangerValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "wing buffet danger") {}

    bool Calculate() override
    {
        Unit* boss = AI_VALUE2(Unit*, "find target", "temporus");
        if (!boss || !boss->IsAlive() || !boss->IsInCombat())
            return false;

        Player* bot = botAI->GetBot();
        if (!bot)
            return false;

        // Wing Buffet knockback - stay at range
        if (boss->FindCurrentSpellBySpellId(SPELL_WING_BUFFET))
        {
            float distance = bot->GetExactDist2d(boss);
            return distance < 10.0f;
        }

        return false;
    }
};

// Mortal Wound Active Value
class MortalWoundActiveValue : public BoolCalculatedValue
{
public:
    MortalWoundActiveValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "mortal wound active") {}

    bool Calculate() override
    {
        Player* bot = botAI->GetBot();
        if (!bot)
            return false;

        // Check if bot has Mortal Wound debuff (reduced healing)
        return bot->HasAura(SPELL_MORTAL_WOUND);
    }
};

// Temporus Spell Reflection Value
class TemporusReflectActiveValue : public BoolCalculatedValue
{
public:
    TemporusReflectActiveValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "temporus reflect active") {}

    bool Calculate() override
    {
        Unit* boss = AI_VALUE2(Unit*, "find target", "temporus");
        if (!boss || !boss->IsAlive() || !boss->IsInCombat())
            return false;

        // Don't cast spells when Temporus has Spell Reflection
        return boss->HasAura(SPELL_SPELL_REFLECTION);
    }
};

#endif