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
#include <set>


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
        {
            return true;
        }
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
bool MoroesAddsTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Exclude healers - WotLK pattern per CLAUDE.md:705
    if (botAI->IsHeal(bot))
        return false;

    // WotLK Pattern: Check if Moroes is in combat first (tank has engaged)
    Unit* moroes = bot->FindNearestCreature(NPC_MOROES, 100.0f, true);
    if (!moroes || !moroes->IsAlive() || !moroes->IsInCombat())
        return false;
    
    // Additional check: Ensure tank has aggro before allowing DPS
    if (!moroes->GetVictim())
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
bool CuratorFlareTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Exclude healers - WotLK pattern per CLAUDE.md:705
    if (botAI->IsHeal(bot))
        return false;

    // CRITICAL: Check Curator combat state first - WotLK standard per CLAUDE.md:251-254
    Unit* curator = bot->FindNearestCreature(NPC_CURATOR, 100.0f, true);
    if (!curator || !curator->IsAlive() || !curator->IsInCombat())
        return false;
    
    // Ensure tank has aggro before allowing DPS - CLAUDE.md:256-258
    if (!curator->GetVictim())
        return false;

    // Only check for flares during actual encounter and within reasonable range
    Unit* flare = bot->FindNearestCreature(NPC_ASTRAL_FLARE, 40.0f, true);
    if (!flare || !flare->IsAlive() || !flare->IsInCombat())
        return false;
    
    // Additional check: ensure flare is accessible (not through walls)
    // Simple line-of-sight check
    if (!bot->IsWithinLOSInMap(flare))
        return false;
        
    return true;
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

// Terestian Illhoof triggers
bool IllhoofDemonChainsTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Exclude healers - WotLK pattern per CLAUDE.md:705
    if (botAI->IsHeal(bot))
        return false;

    // CRITICAL: Check Illhoof combat state first - WotLK standard per CLAUDE.md:251-254
    Unit* illhoof = bot->FindNearestCreature(NPC_TERESTIAN_ILLHOOF, 100.0f, true);
    if (!illhoof || !illhoof->IsAlive() || !illhoof->IsInCombat())
        return false;
    
    // Ensure tank has aggro before allowing DPS - CLAUDE.md:256-258
    if (!illhoof->GetVictim())
        return false;

    // Check for Kilrek first (priority target)
    if (bot->FindNearestCreature(NPC_KILTREK, 100.0f, true))
        return true;

    // Check if Demon Chains exist
    Unit* chains = bot->FindNearestCreature(NPC_DEMON_CHAINS, 100.0f, true);
    if (chains && chains->IsAlive() && chains->IsInCombat())
        return true;
        
    // Check if Illhoof is casting Sacrifice (creates chains)
    if (illhoof->HasUnitState(UNIT_STATE_CASTING))
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

    // Exclude healers - WotLK pattern per CLAUDE.md:705
    if (botAI->IsHeal(bot))
        return false;

    // CRITICAL: Check Illhoof combat state first - WotLK standard per CLAUDE.md:251-254
    Unit* illhoof = bot->FindNearestCreature(NPC_TERESTIAN_ILLHOOF, 100.0f, true);
    if (!illhoof || !illhoof->IsAlive() || !illhoof->IsInCombat())
        return false;
    
    // Ensure tank has aggro before allowing DPS - CLAUDE.md:256-258
    if (!illhoof->GetVictim())
        return false;

    // NOW check for imps (only after combat initiated)
    Unit* imp = bot->FindNearestCreature(NPC_FIENDISH_IMP, 30.0f, true);
    return imp && imp->IsAlive() && imp->IsInCombat();
}

// Netherspite triggers
bool NetherspiteBeamsTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* netherspite = bot->FindNearestCreature(NPC_NETHERSPITE, 100.0f);
    if (!netherspite || !netherspite->IsInCombat())
        return false;
        
    // Only trigger during portal phase (when NOT banished)
    // Banish aura ID = 38524, when this is present, no beams are active
    return !netherspite->HasAura(38524);
}

bool NetherspiteVoidZoneTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if in Void Zone
    if (bot->HasAura(SPELL_VOID_ZONE))
        return true;

    // Generic hazard proximity: Minor Void Zone trigger creature (DB entry 17470)
    if (Unit* hz = bot->FindNearestCreature(17470, 12.0f, true))
        return true;
        
    // Check if Netherspite is casting Void Zone and bot is the target
    Unit* netherspite = bot->FindNearestCreature(NPC_NETHERSPITE, 100.0f);
    if (netherspite)
    {
        // Prefer exact spell match via Unit::FindCurrentSpellBySpellId
        if (Spell* sp = netherspite->FindCurrentSpellBySpellId(SPELL_VOID_ZONE))
        {
            // If the unit target is us, preemptively react
            if (Unit* castTarget = sp->m_targets.GetUnitTarget())
            {
                if (castTarget == bot)
                    return true;
            }
            // If there is a destination position targeted and we are close to it, react as well
            if (SpellDestination const* dst = sp->m_targets.GetDst())
            {
                float dx = bot->GetPositionX() - dst->_position.GetPositionX();
                float dy = bot->GetPositionY() - dst->_position.GetPositionY();
                if ((dx * dx + dy * dy) < (10.0f * 10.0f))
                    return true;
            }
        }
    }
    
    return false;
}

// Prince Malchezaar triggers
bool MalchezaarInfernalTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Exclude healers - WotLK pattern per CLAUDE.md:705
    if (botAI->IsHeal(bot))
        return false;

    // CRITICAL: Check Malchezaar combat state first - WotLK standard per CLAUDE.md:590-592
    Unit* malchezaar = bot->FindNearestCreature(NPC_PRINCE_MALCHEZAAR, 100.0f, true);
    if (!malchezaar || !malchezaar->IsAlive() || !malchezaar->IsInCombat())
        return false;
        
    // Ensure tank has aggro before allowing DPS - CLAUDE.md:595-597
    if (!malchezaar->GetVictim())
        return false;

    // SPAWNED ADD DETECTION: Use dynamic detection per CLAUDE.md:602-603
    const GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (const auto& guid : targets) {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_NETHERSPITE_INFERNAL && unit->IsInCombat()) {
            return true; // Found spawned infernal
        }
    }
    return false;
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
bool NightbaneAirPhaseTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* nightbane = bot->FindNearestCreature(NPC_NIGHTBANE, 100.0f);
    if (!nightbane)
        return false;
    
    // Only react to Nightbane mechanics during the actual encounter
    if (!nightbane->IsInCombat())
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

bool NightbaneSkeletonTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Exclude healers - WotLK pattern per CLAUDE.md:705
    if (botAI->IsHeal(bot))
        return false;

    // CRITICAL: Check Nightbane combat state first - WotLK standard per CLAUDE.md:590-592
    Unit* nightbane = bot->FindNearestCreature(NPC_NIGHTBANE, 100.0f, true);
    if (!nightbane || !nightbane->IsAlive() || !nightbane->IsInCombat())
        return false;
        
    // Ensure tank has aggro before allowing DPS - CLAUDE.md:595-597
    if (!nightbane->GetVictim())
        return false;

    // SPAWNED ADD DETECTION: Use dynamic detection per CLAUDE.md:602-603
    const GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (const auto& guid : targets) {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_RESTLESS_SKELETON && unit->IsInCombat()) {
            return true; // Found spawned skeleton
        }
    }
    return false;
}

// Chess Event triggers
bool ChessEventActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // First check if bot is already possessing a chess piece (priority check)
    if (bot->GetVehicleBase())
        return true;

    // Check if Echo of Medivh is present (Chess Event controller)
    Unit* medivh = bot->FindNearestCreature(NPC_ECHO_OF_MEDIVH, 100.0f);
    if (!medivh)
        return false;

    // Check if chess event is in progress - multiple detection methods
    if (medivh->HasAura(SPELL_GAME_IN_SESSION))
        return true;
        
    // Check if any chess pieces exist nearby (alternative detection)
    if (bot->FindNearestCreature(NPC_CHESS_KING_LLANE, 50.0f) ||
        bot->FindNearestCreature(NPC_WARCHIEF_BLACKHAND, 50.0f) ||
        bot->FindNearestCreature(NPC_HUMAN_FOOTMAN, 50.0f) ||
        bot->FindNearestCreature(NPC_ORC_GRUNT, 50.0f))
        return true;

    return false;
}
