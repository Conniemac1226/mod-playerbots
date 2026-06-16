#include "SteamvaultTriggers.h"
#include "Group.h"
#include "Unit.h"
#include "Playerbots.h"

namespace
{
bool HasAttackableDungeonAdd(PlayerbotAI* botAI, Player* bot, char const* bossName, uint32 addEntry,
    bool requireSelectable = false)
{
    if (!bot || !botAI)
        return false;

    AiObjectContext* context = botAI->GetAiObjectContext();
    if (!context)
        return false;

    Unit* boss = context->GetValue<Unit*>("find target", bossName)->Get();
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    GuidVector targets = context->GetValue<GuidVector>("possible targets no los")->Get();
    for (ObjectGuid const& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (!unit || !unit->IsAlive() || unit->GetEntry() != addEntry)
            continue;

        if (requireSelectable && unit->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            continue;

        return true;
    }

    return false;
}
}

bool HasAttackableThespiaWaterElemental(PlayerbotAI* botAI, Player* bot)
{
    return HasAttackableDungeonAdd(botAI, bot, "hydromancer thespia", NPC_THESPIA_WATER_ELEMENTAL);
}

bool HasAttackableSteamriggerMechanic(PlayerbotAI* botAI, Player* bot)
{
    return HasAttackableDungeonAdd(botAI, bot, "mekgineer steamrigger", NPC_STEAMRIGGER_MECHANIC);
}

bool HasAttackableKalithreshDistiller(PlayerbotAI* botAI, Player* bot)
{
    return HasAttackableDungeonAdd(botAI, bot, "warlord kalithresh", NPC_NAGA_DISTILLER, true);
}

// Hydromancer Thespia
bool ThespiaLightningCloudTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_HYDROMANCER_THESPIA, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if boss is casting Lightning Cloud - immediate detection
    return boss->FindCurrentSpellBySpellId(SPELL_LIGHTNING_CLOUD);
}

bool ThespiaLungBurstTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    if (bot->HasAura(SPELL_LUNG_BURST))
        return true;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* memberRef = group->GetFirstMember(); memberRef != nullptr; memberRef = memberRef->next())
    {
        Player* member = memberRef->GetSource();
        if (member && member->IsAlive() && member->HasAura(SPELL_LUNG_BURST))
            return true;
    }

    return false;
}

bool ThespiaWaterElementalActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    return HasAttackableThespiaWaterElemental(botAI, bot);
}

// Mekgineer Steamrigger
bool SteamriggerShrinkRayTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if bot has shrink ray debuff
    return bot->HasAura(SPELL_SUPER_SHRINK_RAY);
}

bool SteamriggerSawBladeTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_MEKGINEER_STEAMRIGGER, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if boss is casting Saw Blade - immediate detection
    return boss->FindCurrentSpellBySpellId(SPELL_SAW_BLADE);
}

bool SteamriggerElectrifiedNetTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if bot has electrified net debuff
    return bot->HasAura(SPELL_ELECTRIFIED_NET);
}

bool SteamriggerMechanicActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    return HasAttackableSteamriggerMechanic(botAI, bot);
}

// Warlord Kalithresh
bool KalithreshSpellReflectionTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_WARLORD_KALITHRESH, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if boss has spell reflection buff
    return boss->HasAura(SPELL_SPELL_REFLECTION);
}

bool KalithreshImpaleTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    if (bot->HasAura(SPELL_IMPALE))
        return true;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* memberRef = group->GetFirstMember(); memberRef != nullptr; memberRef = memberRef->next())
    {
        Player* member = memberRef->GetSource();
        if (member && member->IsAlive() && member->HasAura(SPELL_IMPALE))
            return true;
    }

    return false;
}

bool KalithreshNagaDistillerActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    return HasAttackableKalithreshDistiller(botAI, bot);
}

bool KalithreshChannelingRageTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI)
        return false;

    const GuidVector npcs = AI_VALUE(GuidVector, "possible targets no los");

    for (ObjectGuid const& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_NAGA_DISTILLER)
        {
            if (unit->HasUnitState(UNIT_STATE_CASTING) && unit->FindCurrentSpellBySpellId(SPELL_WARLORDS_RAGE_DISTILLER))
                return true;
        }
    }
    return false;
}

bool KalithreshSpellReflectionEndedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_WARLORD_KALITHRESH, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    Value<bool>* spellReflectionActiveValue = botAI->GetAiObjectContext()->GetValue<bool>("spell reflection active");
    if (!spellReflectionActiveValue)
        return false;

    bool wasActive = spellReflectionActiveValue->Get();
    bool currentlyActive = boss->HasAura(SPELL_SPELL_REFLECTION);
    
    // Trigger when spell reflection WAS active but is no longer active
    return wasActive && !currentlyActive;
}
