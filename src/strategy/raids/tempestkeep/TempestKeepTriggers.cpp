#include "TempestKeepTriggers.h"
#include "TempestKeepActions.h"
#include "PlayerbotAI.h"
#include "Player.h"
#include "Unit.h"
#include "SpellAuras.h"

// Al'ar triggers
bool AlarFlameQuillsTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Check if Al'ar is casting Flame Quills (34229)
    if (boss->FindCurrentSpellBySpellId(SPELL_FLAME_QUILLS))
    {
        return true;
    }

    // Also check if moving to quill position (platform 6 in the script)
    float quillDist = boss->GetDistance2d(TK_ALAR_QUILL_POSITION.GetPositionX(), 
                                          TK_ALAR_QUILL_POSITION.GetPositionY());
    if (quillDist < 5.0f && boss->GetPositionZ() > 40.0f)
    {
        return true;
    }

    return false;
}

bool AlarDiveBombTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Check for Dive Bomb visual or actual cast
    if (boss->HasAura(SPELL_DIVE_BOMB_VISUAL) || 
        boss->FindCurrentSpellBySpellId(SPELL_DIVE_BOMB))
    {
        return true;
    }

    // Check if boss is at dive position and invisible (dive prep)
    if (!boss->IsVisible())
    {
        float diveDist = boss->GetDistance2d(TK_ALAR_DIVE_POSITION.GetPositionX(),
                                             TK_ALAR_DIVE_POSITION.GetPositionY());
        if (diveDist < 10.0f)
        {
            return true;
        }
    }

    return false;
}

bool AlarFlamePatchTrigger::IsActive()
{
    // Check for nearby flame patches
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_FLAME_PATCH)
        {
            float distance = bot->GetDistance(unit);
            if (distance < 8.0f)
            {
                return true;
            }
        }
    }

    return false;
}

bool AlarPlatformTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Phase 1: Check if Al'ar is on a platform (not in center)
    float centerDist = boss->GetDistance2d(TK_ALAR_CENTER_POSITION.GetPositionX(),
                                           TK_ALAR_CENTER_POSITION.GetPositionY());
    
    // If boss is in center (phase 2), don't need platform movement
    if (centerDist < 10.0f && boss->GetPositionZ() < 5.0f)
    {
        return false;
    }

    // Check if we're in phase 1 (boss on platforms)
    if (boss->GetPositionZ() > 15.0f)
    {
        // Boss is on a platform, we should be too
        return bot->GetPositionZ() < 15.0f;
    }

    return false;
}

bool AlarAddsTrigger::IsActive()
{
    // Check for Ember of Al'ar adds
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_EMBER_OF_ALAR && unit->IsAlive())
        {
            return true;
        }
    }

    return false;
}

// Void Reaver triggers
bool VoidReaverPoundingTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Check if Void Reaver is casting Pounding
    return boss->FindCurrentSpellBySpellId(SPELL_POUNDING) != nullptr;
}

bool VoidReaverArcaneOrbTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Always active to maintain spreading
    float distance = bot->GetDistance(boss);
    return distance > 15.0f && distance < 40.0f;
}

bool VoidReaverPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Check positioning requirements
    bool isMainTank = botAI->IsMainTank(bot);
    float distance = bot->GetDistance(boss);
    
    if (isMainTank)
    {
        // Tank needs to be in position after knock away
        return distance > 5.0f;
    }
    else
    {
        // Ranged need proper positioning
        return distance < 20.0f || distance > 35.0f;
    }
}

// Solarian triggers
bool SolarianWrathTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Check for Wrath of the Astromancer debuff
    return bot->HasAura(SPELL_WRATH_OF_THE_ASTROMANCER);
}

bool SolarianBlindingLightTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if boss is casting Blinding Light
    return boss->FindCurrentSpellBySpellId(SPELL_BLINDING_LIGHT);
}

bool SolarianPortalTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!boss || !boss->IsAlive())
        return false;

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
    // Check for Solarian adds
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
            
        uint32 entry = unit->GetEntry();
        if (entry == NPC_SOLARIUM_AGENT || entry == NPC_SOLARIUM_PRIEST)
        {
            return true;
        }
    }

    return false;
}

// Kael'thas triggers
bool KaelthasAdvisorsTrigger::IsActive()
{
    // Check for advisor NPCs
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
            
        uint32 entry = unit->GetEntry();
        if (entry == NPC_THALADRED || entry == NPC_LORD_SANGUINAR ||
            entry == NPC_GRAND_ASTROMANCER || entry == NPC_MASTER_ENGINEER)
        {
            return true;
        }
    }

    return false;
}

bool KaelthasWeaponsTrigger::IsActive()
{
    // Check if weapons phase is active
    // Weapons are typically targetable objects with specific names
    Unit* boss = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // During weapons phase, Kael'thas is typically immune
    if (boss->HasAura(SPELL_TK_SHOCK_BARRIER))
    {
        return true;
    }

    return false;
}

bool KaelthasPhoenixTrigger::IsActive()
{
    // Check for Phoenix or Phoenix Egg
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
            
        uint32 entry = unit->GetEntry();
        if ((entry == NPC_TK_PHOENIX || entry == NPC_TK_PHOENIX_EGG) && unit->IsAlive())
        {
            return true;
        }
    }

    return false;
}

bool KaelthasFlamestrikeTrigger::IsActive()
{
    // Check for Flamestrike indicators
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_TK_FLAMESTRIKE)
        {
            float distance = bot->GetDistance(unit);
            if (distance < 10.0f)
            {
                return true;
            }
        }
    }

    // Also check if boss is casting flamestrike
    Unit* boss = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (boss && boss->FindCurrentSpellBySpellId(SPELL_KAELTHAS_FLAMESTRIKE))
    {
        return true;
    }

    return false;
}

bool KaelthasGravityLapseTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Check if we have gravity lapse debuff
    if (bot->HasAura(SPELL_TK_GRAVITY_LAPSE))
    {
        return true;
    }

    // Check if boss is casting it
    return boss->FindCurrentSpellBySpellId(SPELL_TK_GRAVITY_LAPSE) != nullptr;
}

bool KaelthasPyroblastTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if boss is casting Pyroblast
    return boss->FindCurrentSpellBySpellId(SPELL_TK_PYROBLAST);
}

bool KaelthasMindControlTrigger::IsActive()
{
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
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (!thaladred || !thaladred->IsAlive())
        return false;

    // Check if Thaladred is fixated on us
    return thaladred->GetTarget() == bot->GetGUID();
}

bool CapernianConflagrationTrigger::IsActive()
{
    // Check if we have Conflagration debuff
    return bot->HasAura(SPELL_CONFLAGRATION);
}

bool TelonicusRemoteToyTrigger::IsActive()
{
    // Check if we have Remote Toy debuff
    return bot->HasAura(SPELL_REMOTE_TOY);
}