#include "KarazhanTriggers.h"
#include "KarazhanActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Player.h"
#include "Unit.h"
#include "Creature.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "Group.h"
#include "GroupReference.h"

// Helper function to check if a unit is casting a specific spell
static bool IsCastingSpell(Unit* unit, uint32 spellId)
{
    if (!unit || !unit->HasUnitState(UNIT_STATE_CASTING))
        return false;
        
    // Check all spell types (melee, generic, channeled, autorepeat)
    for (uint32 i = CURRENT_MELEE_SPELL; i <= CURRENT_CHANNELED_SPELL; ++i)
    {
        CurrentSpellTypes spellType = CurrentSpellTypes(i);
        if (Spell* spell = unit->GetCurrentSpell(spellType))
        {
            if (spell->m_spellInfo->Id == spellId)
                return true;
        }
    }
    return false;
}

// Helper to check multiple spell IDs
static bool IsCastingAnySpell(Unit* unit, std::initializer_list<uint32> spellIds)
{
    for (uint32 spellId : spellIds)
    {
        if (IsCastingSpell(unit, spellId))
            return true;
    }
    return false;
}

bool AttumenEngagedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Attumen has spawned (phase 2 started)
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_ATTUMEN_UNMOUNTED && unit->IsInCombat())
            return true;
    }
    
    return false;
}

bool AttumenMountedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if the mounted version exists
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_ATTUMEN_MOUNTED)
            return true;
    }
    
    return false;
}

bool AttumenChargeDangerTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if mounted Attumen exists and we're in the danger zone
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_ATTUMEN_MOUNTED)
        {
            float distance = bot->GetDistance(unit);
            // Charge targets players between 8-25 yards
            if (distance > 8.0f && distance < 25.0f)
                return true;
        }
    }
    
    return false;
}

bool AttumenShadowcleaveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if we're in front of Attumen (either form)
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 100.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 100.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        // Check both unmounted and mounted versions
        if (unit->GetEntry() == NPC_ATTUMEN_UNMOUNTED || unit->GetEntry() == NPC_ATTUMEN_MOUNTED)
        {
            // If we're close and in front, we're in danger of shadowcleave
            if (bot->GetDistance(unit) < 10.0f && !unit->HasInArc(M_PI / 2, bot))
                return true;
        }
    }
    
    return false;
}

// Moroes triggers
bool MoroesEngagedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* moroes = bot->FindNearestCreature(NPC_MOROES, 100.0f, true);
    if (!moroes)
        return false;
        
    // Only activate when Moroes is actually in combat, not just present
    return moroes->IsInCombat() && moroes->GetVictim() != nullptr;
}

bool MoroesAddsTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    uint32 addIds[] = {
        NPC_BARONESS_DOROTHEA, NPC_LADY_CATRIONA, NPC_LADY_KEIRA,
        NPC_LORD_ROBIN, NPC_LORD_CRISPIN, NPC_BARON_RAFE
    };

    for (uint32 npcId : addIds)
    {
        if (bot->FindNearestCreature(npcId, 100.0f, true))
            return true;
    }
    
    return false;
}

bool MoroesGarroteTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if already garrotted
    if (bot->HasAura(SPELL_GARROTE))
        return true;
        
    // Check if Moroes is about to vanish (precedes Garrote)
    Unit* moroes = bot->FindNearestCreature(NPC_MOROES, 100.0f);
    if (moroes && IsCastingSpell(moroes, SPELL_VANISH))
        return true; // Vanish precedes Garrote
    
    return false;
}

// Maiden of Virtue triggers
bool MaidenEngagedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    return bot->FindNearestCreature(NPC_MAIDEN_OF_VIRTUE, 100.0f, true) != nullptr;
}

bool MaidenRepentanceTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if we have repentance or if allies nearby have it (to spread out)
    if (bot->HasAura(SPELL_REPENTANCE))
        return true;
        
    // Check if Maiden is casting Repentance
    Unit* maiden = bot->FindNearestCreature(NPC_MAIDEN_OF_VIRTUE, 100.0f);
    if (maiden && maiden->HasUnitState(UNIT_STATE_CASTING))
    {
        CurrentSpellTypes spellType = CURRENT_GENERIC_SPELL;
        if (maiden->GetCurrentSpell(spellType))
        {
            uint32 spellId = maiden->GetCurrentSpell(spellType)->m_spellInfo->Id;
            if (spellId == SPELL_REPENTANCE)
                return true; // Spread out before cast completes
        }
    }

    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member != bot && member->GetDistance(bot) < 8.0f)
            {
                if (member->HasAura(SPELL_REPENTANCE))
                    return true;
            }
        }
    }
    
    return false;
}

bool MaidenHolyGroundTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if standing in Holy Ground
    if (bot->HasAura(SPELL_HOLY_GROUND))
        return true;
        
    // Check if Maiden is casting Holy Ground
    Unit* maiden = bot->FindNearestCreature(NPC_MAIDEN_OF_VIRTUE, 100.0f);
    if (maiden && maiden->HasUnitState(UNIT_STATE_CASTING))
    {
        CurrentSpellTypes spellType = CURRENT_GENERIC_SPELL;
        if (maiden->GetCurrentSpell(spellType))
        {
            uint32 spellId = maiden->GetCurrentSpell(spellType)->m_spellInfo->Id;
            // Holy Ground is typically cast at current location
            if (spellId == SPELL_HOLY_GROUND && bot->GetDistance(maiden) < 10.0f)
                return true;
        }
    }
    
    return false;
}

// Opera Event triggers
bool OperaEngagedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    uint32 operaNpcs[] = {
        NPC_DOROTHEE, NPC_ROAR, NPC_STRAWMAN, NPC_TINHEAD, NPC_CRONE,
        NPC_ROMULO, NPC_JULIANNE, NPC_BIG_BAD_WOLF
    };

    for (uint32 npcId : operaNpcs)
    {
        Unit* operaNpc = bot->FindNearestCreature(npcId, 100.0f, true);
        if (operaNpc && operaNpc->IsInCombat() && operaNpc->GetVictim())
            return true;
    }
    
    return false;
}

bool OperaOzEngagedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    uint32 ozNpcs[] = {
        NPC_DOROTHEE, NPC_ROAR, NPC_STRAWMAN, NPC_TINHEAD, NPC_CRONE
    };

    for (uint32 npcId : ozNpcs)
    {
        Unit* operaNpc = bot->FindNearestCreature(npcId, 100.0f, true);
        if (operaNpc && operaNpc->IsInCombat() && operaNpc->GetVictim())
            return true;
    }
    
    return false;
}

bool OperaRomuloJulianneTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* romulo = bot->FindNearestCreature(NPC_ROMULO, 100.0f, true);
    Unit* julianne = bot->FindNearestCreature(NPC_JULIANNE, 100.0f, true);
    
    return (romulo && romulo->IsInCombat() && romulo->GetVictim()) ||
           (julianne && julianne->IsInCombat() && julianne->GetVictim());
}

bool OperaWolfTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if we have Red Riding Hood debuff and wolf is in combat
    Unit* wolf = bot->FindNearestCreature(NPC_BIG_BAD_WOLF, 100.0f, true);
    return bot->HasAura(30753) && wolf && wolf->IsInCombat() && wolf->GetVictim();
}

// Curator triggers
bool CuratorEngagedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    return bot->FindNearestCreature(NPC_CURATOR, 100.0f, true) != nullptr;
}

bool CuratorFlareTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    return bot->FindNearestCreature(NPC_ASTRAL_FLARE, 100.0f, true) != nullptr;
}

bool CuratorEvocationTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* curator = bot->FindNearestCreature(NPC_CURATOR, 100.0f);
    if (!curator)
        return false;
        
    // Check if already in evocation
    if (curator->HasAura(SPELL_CURATOR_EVOCATION))
        return true;
        
    // Check if starting to cast evocation (check both channeled and generic)
    if (curator->HasUnitState(UNIT_STATE_CASTING))
    {
        // Check channeled spells first
        for (uint32 i = CURRENT_MELEE_SPELL; i <= CURRENT_CHANNELED_SPELL; ++i)
        {
            CurrentSpellTypes spellType = CurrentSpellTypes(i);
            if (curator->GetCurrentSpell(spellType))
            {
                uint32 spellId = curator->GetCurrentSpell(spellType)->m_spellInfo->Id;
                if (spellId == SPELL_CURATOR_EVOCATION)
                    return true;
            }
        }
    }
    
    return false;
}

// Shade of Aran triggers
bool AranEngagedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    return bot->FindNearestCreature(NPC_SHADE_OF_ARAN, 100.0f, true) != nullptr;
}

bool AranFlameWreathTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if we already have Flame Wreath
    if (bot->HasAura(SPELL_FLAME_WREATH))
        return true;
        
    // Check if Aran is casting Flame Wreath (preemptive detection)
    Unit* aran = bot->FindNearestCreature(NPC_SHADE_OF_ARAN, 100.0f);
    if (aran && IsCastingSpell(aran, SPELL_FLAME_WREATH))
        return true;
    
    return false;
}

bool AranBlizzardTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if already in Blizzard
    if (bot->HasAura(SPELL_CIRCULAR_BLIZZARD))
        return true;
        
    // Check if Aran is casting Circular Blizzard
    Unit* aran = bot->FindNearestCreature(NPC_SHADE_OF_ARAN, 100.0f);
    if (aran && aran->HasUnitState(UNIT_STATE_CASTING))
    {
        CurrentSpellTypes spellType = CURRENT_GENERIC_SPELL;
        if (aran->GetCurrentSpell(spellType))
        {
            uint32 spellId = aran->GetCurrentSpell(spellType)->m_spellInfo->Id;
            if (spellId == SPELL_CIRCULAR_BLIZZARD)
                return true; // Move to center before it starts
        }
    }
    
    return false;
}

bool AranDragonsBreathTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* aran = bot->FindNearestCreature(NPC_SHADE_OF_ARAN, 100.0f);
    if (!aran)
        return false;

    // Check if casting Dragon's Breath specifically
    // Dragon's Breath is frontal cone, only care if we're in front and close
    if (IsCastingSpell(aran, SPELL_DRAGONS_BREATH) && bot->GetDistance(aran) < 15.0f)
        return true;
    
    return false;
}

bool AranArcanExplosionTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* aran = bot->FindNearestCreature(NPC_SHADE_OF_ARAN, 100.0f);
    if (!aran)
        return false;

    // Arcane Explosion at low health - check if Aran is casting it
    if (aran->GetHealthPct() < 40.0f && aran->HasUnitState(UNIT_STATE_CASTING))
    {
        CurrentSpellTypes spellType = CURRENT_GENERIC_SPELL;
        if (aran->GetCurrentSpell(spellType))
        {
            uint32 spellId = aran->GetCurrentSpell(spellType)->m_spellInfo->Id;
            // Arcane Explosion spell ID (29973)
            if (spellId == 29973 && bot->GetDistance(aran) < 10.0f)
                return true;
        }
    }
    
    return false;
}

bool AranMassPolymorphTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* aran = bot->FindNearestCreature(NPC_SHADE_OF_ARAN, 100.0f);
    if (!aran)
        return false;

    // Check if Aran is casting Mass Polymorph
    // This spell usually has a distinctive cast time
    if (IsCastingSpell(aran, SPELL_MASS_POLYMORPH))
        return true;
    
    // Also check if we're already polymorphed
    if (bot->HasAura(SPELL_MASS_POLYMORPH))
        return true;
    
    return false;
}

// Terestian Illhoof triggers
bool IllhoofEngagedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    return bot->FindNearestCreature(NPC_TERESTIAN_ILLHOOF, 100.0f, true) != nullptr;
}

bool IllhoofDemonChainsTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Demon Chains exist
    if (bot->FindNearestCreature(NPC_DEMON_CHAINS, 100.0f, true))
        return true;
        
    // Check if Illhoof is casting Sacrifice (creates chains)
    Unit* illhoof = bot->FindNearestCreature(NPC_TERESTIAN_ILLHOOF, 100.0f);
    if (illhoof && illhoof->HasUnitState(UNIT_STATE_CASTING))
    {
        CurrentSpellTypes spellType = CURRENT_GENERIC_SPELL;
        if (illhoof->GetCurrentSpell(spellType))
        {
            uint32 spellId = illhoof->GetCurrentSpell(spellType)->m_spellInfo->Id;
            if (spellId == SPELL_SACRIFICE || spellId == SPELL_SUMMON_DEMONCHAINS)
                return true;
        }
    }
    
    return false;
}

bool IllhoofImpsTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    return bot->FindNearestCreature(NPC_FIENDISH_IMP, 30.0f, true) != nullptr;
}

// Netherspite triggers
bool NetherspiteEngagedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    return bot->FindNearestCreature(NPC_NETHERSPITE, 100.0f, true) != nullptr;
}

bool NetherspiteBeamsTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* netherspite = bot->FindNearestCreature(NPC_NETHERSPITE, 100.0f);
    // Simplified - would need actual beam detection
    return netherspite && netherspite->IsInCombat();
}

bool NetherspiteVoidZoneTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if in Void Zone
    if (bot->HasAura(SPELL_VOID_ZONE))
        return true;
        
    // Check if Netherspite is casting Void Zone
    Unit* netherspite = bot->FindNearestCreature(NPC_NETHERSPITE, 100.0f);
    if (netherspite && netherspite->HasUnitState(UNIT_STATE_CASTING))
    {
        CurrentSpellTypes spellType = CURRENT_GENERIC_SPELL;
        if (netherspite->GetCurrentSpell(spellType))
        {
            uint32 spellId = netherspite->GetCurrentSpell(spellType)->m_spellInfo->Id;
            // Void Zone is typically placed at target location
            if (spellId == SPELL_VOID_ZONE && netherspite->GetVictim() == bot)
                return true; // Move preemptively if targeted
        }
    }
    
    return false;
}

// Prince Malchezaar triggers
bool MalchezaarEngagedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    return bot->FindNearestCreature(NPC_PRINCE_MALCHEZAAR, 100.0f, true) != nullptr;
}

bool MalchezaarInfernalTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    return bot->FindNearestCreature(NPC_NETHERSPITE_INFERNAL, 15.0f, true) != nullptr;
}

bool MalchezaarEnfeebleTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if already enfeebled
    if (bot->HasAura(30843))
        return true;
        
    // Check if Malchezaar is casting Enfeeble
    Unit* malchezaar = bot->FindNearestCreature(NPC_PRINCE_MALCHEZAAR, 100.0f);
    if (malchezaar && malchezaar->HasUnitState(UNIT_STATE_CASTING))
    {
        CurrentSpellTypes spellType = CURRENT_GENERIC_SPELL;
        if (malchezaar->GetCurrentSpell(spellType))
        {
            uint32 spellId = malchezaar->GetCurrentSpell(spellType)->m_spellInfo->Id;
            if (spellId == 30843) // Enfeeble cast
                return true;
        }
    }
    
    return false;
}

// Nightbane triggers
bool NightbaneEngagedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    return bot->FindNearestCreature(NPC_NIGHTBANE, 100.0f, true) != nullptr;
}

bool NightbaneAirPhaseTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* nightbane = bot->FindNearestCreature(NPC_NIGHTBANE, 100.0f);
    if (!nightbane)
        return false;
        
    // Air phase is when Nightbane is out of melee range
    if (!nightbane->IsWithinMeleeRange(bot))
        return true;
        
    // Also check if Nightbane is casting Rain of Bones (air phase ability)
    if (nightbane->HasUnitState(UNIT_STATE_CASTING))
    {
        CurrentSpellTypes spellType = CURRENT_GENERIC_SPELL;
        if (nightbane->GetCurrentSpell(spellType))
        {
            uint32 spellId = nightbane->GetCurrentSpell(spellType)->m_spellInfo->Id;
            if (spellId == SPELL_RAIN_OF_BONES || spellId == SPELL_FIREBALL_BARRAGE)
                return true;
        }
    }
    
    return false;
}

bool NightbaneCharredEarthTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    return bot->HasAura(SPELL_CHARRED_EARTH);
}