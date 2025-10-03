#include "TempestKeepTriggers.h"
#include "TempestKeepActions.h"
#include "PlayerbotAI.h"
#include "Player.h"
#include "Unit.h"
#include "SpellAuras.h"

// Al'ar triggers - ICC PATTERN: Simple existence checks
bool AlarFlameQuillsTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss) return false;

    // ICC Pattern: Simple check without combat validation
    if (boss->FindCurrentSpellBySpellId(SPELL_FLAME_QUILLS))
        return true;

    // Also check if moving to quill position
    float quillDist = boss->GetDistance2d(TK_ALAR_QUILL_POSITION.GetPositionX(),
                                          TK_ALAR_QUILL_POSITION.GetPositionY());
    if (quillDist < 5.0f && boss->GetPositionZ() > 40.0f)
        return true;

    return false;
}

bool AlarDiveBombTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss) return false;

    // ICC Pattern: Simple existence check
    if (boss->HasAura(SPELL_DIVE_BOMB_VISUAL) ||
        boss->FindCurrentSpellBySpellId(SPELL_DIVE_BOMB))
        return true;

    // Check if boss is invisible (dive prep)
    if (!boss->IsVisible())
    {
        float diveDist = boss->GetDistance2d(TK_ALAR_DIVE_POSITION.GetPositionX(),
                                             TK_ALAR_DIVE_POSITION.GetPositionY());
        if (diveDist < 10.0f)
            return true;
    }

    return false;
}

bool AlarFlamePatchTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    // ICC Pattern: Simple spawned add detection
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_FLAME_PATCH)
        {
            float distance = bot->GetDistance(unit);
            if (distance < 8.0f)
                return true;
        }
    }

    return false;
}

bool AlarPlatformTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss) return false;

    // Phase 1: Check if Al'ar is on a platform (not in center)
    float centerDist = boss->GetDistance2d(TK_ALAR_CENTER_POSITION.GetPositionX(),
                                           TK_ALAR_CENTER_POSITION.GetPositionY());

    // If boss is in center (phase 2), trigger platform ejection
    if (centerDist < 10.0f && boss->GetPositionZ() < 5.0f)
    {
        // Melee and tanks need to jump off
        if (botAI->IsTank(bot) || botAI->IsMelee(bot))
            return bot->GetPositionZ() > 5.0f;
        return false;
    }

    // Phase 1: Boss on platforms - only tanks and melee go up
    if (boss->GetPositionZ() > 15.0f)
    {
        // Ranged DPS should NOT be on platforms
        if (botAI->IsRanged(bot) && !botAI->IsHeal(bot))
            return false;

        // Tanks and melee need to be on platform
        if (botAI->IsTank(bot) || botAI->IsMelee(bot))
            return bot->GetPositionZ() < 15.0f;
    }

    return false;
}

bool AlarAddsTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    // ICC Pattern: Spawned add detection
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_EMBER_OF_ALAR && unit->IsAlive())
            return true;
    }

    return false;
}

// Void Reaver triggers - ICC PATTERN
bool VoidReaverPoundingTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!boss) return false;

    // ICC Pattern: Simple spell check
    return boss->FindCurrentSpellBySpellId(SPELL_POUNDING) != nullptr;
}

bool VoidReaverArcaneOrbTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!boss) return false;

    // Trigger spread mechanic when in combat
    float distance = bot->GetDistance(boss);
    return distance > 15.0f && distance < 40.0f;
}

bool VoidReaverPositionTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    // Only for ranged - tanks handled by normal combat routine
    if (botAI->IsTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!boss) return false;

    // Ranged need proper positioning (20-35 yards)
    float distance = bot->GetDistance(boss);
    return distance < 20.0f || distance > 35.0f;
}

// Solarian triggers - ICC PATTERN
bool SolarianWrathTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    // ICC Pattern: Simple aura check
    return bot->HasAura(SPELL_WRATH_OF_THE_ASTROMANCER);
}

bool SolarianBlindingLightTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!boss) return false;

    // ICC Pattern: Simple spell check
    return boss->FindCurrentSpellBySpellId(SPELL_BLINDING_LIGHT) != nullptr;
}

bool SolarianPortalTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!boss) return false;

    // Boss disappears during portal phase
    if (!boss->IsVisible())
        return true;

    // Check for spotlight creatures
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_ASTROMANCER_SOLARIAN_SPOTLIGHT)
            return true;
    }

    return false;
}

bool SolarianAddsTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    // ICC Pattern: Spawned add detection
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        uint32 entry = unit->GetEntry();
        if (entry == NPC_SOLARIUM_AGENT || entry == NPC_SOLARIUM_PRIEST)
            return true;
    }

    return false;
}

// Kael'thas triggers - ICC PATTERN
bool KaelthasAdvisorsTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    // ICC Pattern: Pre-existing adds use direct targeting
    if (AI_VALUE2(Unit*, "find target", "thaladred the darkener"))
        return true;
    if (AI_VALUE2(Unit*, "find target", "lord sanguinar"))
        return true;
    if (AI_VALUE2(Unit*, "find target", "grand astromancer capernian"))
        return true;
    if (AI_VALUE2(Unit*, "find target", "master engineer telonicus"))
        return true;

    return false;
}

bool KaelthasWeaponsTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!boss) return false;

    // ICC Pattern: Simple aura check for weapons phase
    return boss->HasAura(SPELL_TK_SHOCK_BARRIER);
}

bool KaelthasPhoenixTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    // ICC Pattern: Spawned add detection
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        uint32 entry = unit->GetEntry();
        if ((entry == NPC_TK_PHOENIX || entry == NPC_TK_PHOENIX_EGG) && unit->IsAlive())
            return true;
    }

    return false;
}

bool KaelthasFlamestrikeTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    // Check for Flamestrike indicators
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_TK_FLAMESTRIKE)
        {
            float distance = bot->GetDistance(unit);
            if (distance < 10.0f)
                return true;
        }
    }

    // Also check if boss is casting flamestrike
    Unit* boss = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (boss && boss->FindCurrentSpellBySpellId(SPELL_KAELTHAS_FLAMESTRIKE))
        return true;

    return false;
}

bool KaelthasGravityLapseTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    // ICC Pattern: Simple aura check
    if (bot->HasAura(SPELL_TK_GRAVITY_LAPSE))
        return true;

    // Check if boss is casting it
    Unit* boss = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (boss && boss->FindCurrentSpellBySpellId(SPELL_TK_GRAVITY_LAPSE))
        return true;

    return false;
}

bool KaelthasPyroblastTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!boss) return false;

    // ICC Pattern: Simple spell check
    return boss->FindCurrentSpellBySpellId(SPELL_TK_PYROBLAST) != nullptr;
}

bool KaelthasMindControlTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    // Check if any group member has mind control
    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& guid : members)
    {
        Unit* member = botAI->GetUnit(guid);
        if (member && member->HasAura(SPELL_MIND_CONTROL))
            return true;
    }
    return false;
}

bool KaelthasNetherVaporTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    // Check for nearby Nether Vapor
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_NETHER_VAPOR)
        {
            if (bot->GetDistance(unit) < 15.0f)
                return true;
        }
    }
    return false;
}

bool ThaladredFixateTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (!thaladred || !thaladred->IsAlive())
        return false;

    // Check if Thaladred is fixated on us
    return thaladred->GetTarget() == bot->GetGUID();
}

bool CapernianConflagrationTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    // ICC Pattern: Simple aura check
    return bot->HasAura(SPELL_CONFLAGRATION);
}

bool TelonicusRemoteToyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    // ICC Pattern: Simple aura check
    return bot->HasAura(SPELL_REMOTE_TOY);
}

// New Al'ar triggers for missing mechanics - ICC PATTERN
bool AlarFlameBuffetTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    // Only relevant for tanks
    if (!botAI->IsTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss) return false;

    // Phase 1 only - check if tank has stacking Flame Buffet debuff
    return bot->HasAura(SPELL_FLAME_BUFFET) && boss->GetPositionZ() > 15.0f;
}

bool AlarEmberBlastTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    // ICC Pattern: Check for nearby Ember adds that might cast Ember Blast
    if (Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs"))
    {
        GuidVector const npcs = npcsValue->Get();
        for (ObjectGuid const& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && unit->GetEntry() == NPC_EMBER_OF_ALAR)
            {
                // Check if ember is casting Ember Blast or if we're too close
                if (unit->FindCurrentSpellBySpellId(SPELL_EMBER_BLAST) ||
                    bot->GetDistance(unit) < 8.0f)
                    return true;
            }
        }
    }
    return false;
}

bool AlarMeltArmorTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    // Only relevant for tanks in Phase 2
    if (!botAI->IsTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss) return false;

    // Phase 2 - boss is on ground (Z < 10) and tank has Melt Armor debuff
    return boss->GetPositionZ() < 10.0f && bot->HasAura(SPELL_MELT_ARMOR);
}

bool AlarChargeTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss) return false;

    // Phase 2 - check if boss is casting Charge
    if (boss->GetPositionZ() < 10.0f && boss->FindCurrentSpellBySpellId(SPELL_ALAR_CHARGE))
        return true;

    // Also check if boss is facing us and might charge
    if (boss->GetPositionZ() < 10.0f && boss->HasInArc(M_PI_4, bot))
    {
        float distance = bot->GetDistance(boss);
        return distance > 5.0f && distance < 25.0f;
    }

    return false;
}

bool AlarOfftankPlatformTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    // Only for offtank
    if (!botAI->IsTank(bot) || botAI->IsMainTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss) return false;

    // Phase 1 only - boss on platforms
    if (boss->GetPositionZ() < 15.0f)
        return false;

    // Offtank should be at a different platform than main tank
    // Trigger if offtank is not at proper position
    return true;
}

bool AlarPlateDpsAddTankTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    // Only for plate DPS (warriors, paladins, DKs that are not tanks)
    if (!botAI->IsMelee(bot))
        return false;

    // Check if wearing plate
    uint8 itemClass = 0;
    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            if (item->GetTemplate()->Class == ITEM_CLASS_ARMOR &&
                item->GetTemplate()->SubClass == ITEM_SUBCLASS_ARMOR_PLATE)
            {
                itemClass = ITEM_SUBCLASS_ARMOR_PLATE;
                break;
            }
        }
    }

    if (itemClass != ITEM_SUBCLASS_ARMOR_PLATE)
        return false;

    // Check if Ember of Al'ar adds are present
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_EMBER_OF_ALAR && unit->IsAlive())
            return true;
    }

    return false;
}

bool AlarPlateDpsEscapeTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot) return false;

    // Only for plate DPS tanking adds
    if (!botAI->IsMelee(bot))
        return false;

    // Check for Ember of Al'ar adds that are low health
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_EMBER_OF_ALAR && unit->IsAlive())
        {
            // If add is below 10% health, plate DPS needs to escape
            float healthPct = (unit->GetHealth() * 100.0f) / unit->GetMaxHealth();
            if (healthPct < 10.0f && unit->GetTarget() == bot->GetGUID())
                return true;
        }
    }

    return false;
}
