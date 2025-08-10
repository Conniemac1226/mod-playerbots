#include "GruulTriggers.h"
#include "GruulActions.h"
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

// Gruul the Dragonkiller Triggers
bool GruulEngagedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    return bot->FindNearestCreature(NPC_GRUUL_THE_DRAGONKILLER, 150.0f, true) != nullptr;
}

bool GruulGroundSlamTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* gruul = bot->FindNearestCreature(NPC_GRUUL_THE_DRAGONKILLER, 150.0f);
    if (!gruul)
        return false;
        
    // Check if Gruul is casting Ground Slam (preemptive detection)
    if (IsCastingSpell(gruul, SPELL_GROUND_SLAM))
        return true;
        
    // Check if we're being affected by tractor beam
    if (bot->HasAura(SPELL_TRACTOR_BEAM_PULL))
        return true;
        
    // Check if we have the Look Around stun (during Ground Slam)
    if (bot->HasAura(SPELL_LOOK_AROUND))
        return true;
    
    return false;
}

bool GruulShatterTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* gruul = bot->FindNearestCreature(NPC_GRUUL_THE_DRAGONKILLER, 150.0f);
    if (!gruul)
        return false;
        
    // Check if we're petrified (about to be shattered)
    if (bot->HasAura(SPELL_STONED))
        return true;
        
    // Check if Gruul is casting Shatter (critical to spread out)
    if (IsCastingSpell(gruul, SPELL_SHATTER))
        return true;
        
    // Check if we recently had Ground Slam (shatter follows 9.7 seconds later)
    // This would require tracking Ground Slam timing
    
    return false;
}

bool GruulCaveInTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* gruul = bot->FindNearestCreature(NPC_GRUUL_THE_DRAGONKILLER, 150.0f);
    if (!gruul)
        return false;
        
    // Check if Gruul is casting Cave In
    if (IsCastingSpell(gruul, SPELL_CAVE_IN))
        return true;
    
    // Check if we're in a Cave In area (would need to check for visual/ground effects)
    // Cave In targets random players and creates danger zones
    
    return false;
}

bool GruulReverberationTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if we have Reverberation (silence)
    if (bot->HasAura(SPELL_REVERBERATION))
        return true;
        
    Unit* gruul = bot->FindNearestCreature(NPC_GRUUL_THE_DRAGONKILLER, 150.0f);
    if (!gruul)
        return false;
        
    // Check if Gruul is casting Reverberation
    if (IsCastingSpell(gruul, SPELL_REVERBERATION))
        return true;
    
    return false;
}

bool GruulHurtfulStrikeTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* gruul = bot->FindNearestCreature(NPC_GRUUL_THE_DRAGONKILLER, 150.0f);
    if (!gruul)
        return false;
        
    // Check if we're in melee range and not the main tank
    if (bot->GetDistance(gruul) <= 5.0f && gruul->GetVictim() != bot)
    {
        // Hurtful Strike targets the highest health player in melee range (not main tank)
        Group* group = bot->GetGroup();
        if (group)
        {
            bool highestHealth = true;
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member != bot && member->GetDistance(gruul) <= 5.0f)
                {
                    if (member->GetHealth() > bot->GetHealth() && gruul->GetVictim() != member)
                    {
                        highestHealth = false;
                        break;
                    }
                }
            }
            return highestHealth;
        }
    }
    
    return false;
}

bool GruulGrowthTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* gruul = bot->FindNearestCreature(NPC_GRUUL_THE_DRAGONKILLER, 150.0f);
    if (!gruul)
        return false;
        
    // Check if Gruul has Growth stacks (increases size and damage)
    // Growth happens every 30.3 seconds automatically
    uint32 growthStacks = 0;
    if (Aura* growth = gruul->GetAura(SPELL_GROWTH))
    {
        growthStacks = growth->GetStackAmount();
    }
    
    // Alert if growth is getting high (5+ stacks means high damage)
    return growthStacks >= 5;
}

// High King Maulgar Triggers
bool MaulgarEngagedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    return bot->FindNearestCreature(NPC_HIGH_KING_MAULGAR, 150.0f, true) != nullptr;
}

bool MaulgarCouncilTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    uint32 councilIds[] = {
        NPC_KROSH_FIREHAND,
        NPC_OLM_THE_SUMMONER,
        NPC_KIGGLER_THE_CRAZED,
        NPC_BLINDEYE_THE_SEER
    };
    
    for (uint32 npcId : councilIds)
    {
        if (bot->FindNearestCreature(npcId, 150.0f, true))
            return true;
    }
    
    return false;
}

bool MaulgarWhirlwindTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* maulgar = bot->FindNearestCreature(NPC_HIGH_KING_MAULGAR, 150.0f);
    if (!maulgar)
        return false;
        
    // Check if Maulgar is casting Whirlwind
    if (IsCastingSpell(maulgar, SPELL_WHIRLWIND))
        return true;
        
    // Check if Maulgar has Whirlwind buff active
    if (maulgar->HasAura(SPELL_WHIRLWIND))
        return true;
    
    return false;
}

bool MaulgarArcingSmashTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* maulgar = bot->FindNearestCreature(NPC_HIGH_KING_MAULGAR, 150.0f);
    if (!maulgar)
        return false;
        
    // Check if Maulgar is casting Arcing Smash (frontal cleave)
    if (IsCastingSpell(maulgar, SPELL_ARCING_SMASH))
    {
        // Check if we're in front of Maulgar
        if (!maulgar->HasInArc(M_PI / 2, bot))
            return true;
    }
    
    return false;
}

bool MaulgarBerserkerTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* maulgar = bot->FindNearestCreature(NPC_HIGH_KING_MAULGAR, 150.0f);
    if (!maulgar)
        return false;
        
    // Check if Maulgar has Berserker buff (at 50% health)
    if (maulgar->HasAura(SPELL_BERSERKER_C))
        return true;
        
    // Check if Maulgar is below 50% health (when he gains berserker)
    if (maulgar->GetHealthPct() <= 50.0f)
        return true;
    
    return false;
}

// Council Member Triggers
bool KroshSpellshieldTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* krosh = bot->FindNearestCreature(NPC_KROSH_FIREHAND, 150.0f);
    if (!krosh)
        return false;
        
    // Check if Krosh has Spellshield (needs to be stolen)
    if (krosh->HasAura(SPELL_SPELLSHIELD))
        return true;
        
    // Check if Krosh is casting Spellshield
    if (IsCastingSpell(krosh, SPELL_SPELLSHIELD))
        return true;
    
    return false;
}

bool KroshBlastWaveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* krosh = bot->FindNearestCreature(NPC_KROSH_FIREHAND, 150.0f);
    if (!krosh)
        return false;
        
    // Check if we're too close to Krosh (Blast Wave range is 15 yards)
    if (bot->GetDistance(krosh) < 15.0f)
    {
        // Check if Krosh is casting Blast Wave
        if (IsCastingSpell(krosh, SPELL_BLAST_WAVE))
            return true;
    }
    
    return false;
}

bool KigglerPolymorphTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if we're polymorphed
    if (bot->HasAura(SPELL_GREATER_POLYMORPH))
        return true;
        
    Unit* kiggler = bot->FindNearestCreature(NPC_KIGGLER_THE_CRAZED, 150.0f);
    if (!kiggler)
        return false;
        
    // Check if Kiggler is casting Greater Polymorph
    if (IsCastingSpell(kiggler, SPELL_GREATER_POLYMORPH))
        return true;
    
    return false;
}

bool KigglerArcaneExplosionTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* kiggler = bot->FindNearestCreature(NPC_KIGGLER_THE_CRAZED, 150.0f);
    if (!kiggler)
        return false;
        
    // Check if we're too close to Kiggler
    if (bot->GetDistance(kiggler) < 10.0f)
    {
        // Check if Kiggler is casting Arcane Explosion
        if (IsCastingSpell(kiggler, SPELL_ARCANE_EXPLOSION))
            return true;
    }
    
    return false;
}

bool OlmSummonTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check for Wild Fel Stalker pets
    if (bot->FindNearestCreature(NPC_WILD_FEL_STALKER, 100.0f, true))
        return true;
        
    Unit* olm = bot->FindNearestCreature(NPC_OLM_THE_SUMMONER, 150.0f);
    if (!olm)
        return false;
        
    // Check if Olm is summoning
    if (IsCastingSpell(olm, SPELL_SUMMON_WFH))
        return true;
    
    return false;
}

bool BlindeyeHealTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* blindeye = bot->FindNearestCreature(NPC_BLINDEYE_THE_SEER, 150.0f);
    if (!blindeye)
        return false;
        
    // Check if Blindeye is casting Heal (should be interrupted)
    if (IsCastingSpell(blindeye, SPELL_HEAL))
        return true;
        
    // Check if Blindeye is casting Prayer of Healing
    if (IsCastingSpell(blindeye, SPELL_PRAYER_OH))
        return true;
    
    return false;
}

bool BlindeyeShieldTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* blindeye = bot->FindNearestCreature(NPC_BLINDEYE_THE_SEER, 150.0f);
    if (!blindeye)
        return false;
        
    // Check if any council member has Greater Power Word: Shield
    uint32 councilIds[] = {
        NPC_HIGH_KING_MAULGAR,
        NPC_KROSH_FIREHAND,
        NPC_OLM_THE_SUMMONER,
        NPC_KIGGLER_THE_CRAZED,
        NPC_BLINDEYE_THE_SEER
    };
    
    for (uint32 npcId : councilIds)
    {
        if (Unit* councilMember = bot->FindNearestCreature(npcId, 150.0f))
        {
            if (councilMember->HasAura(SPELL_GREATER_PW_SHIELD))
                return true;
        }
    }
    
    // Check if Blindeye is casting Greater Power Word: Shield
    if (IsCastingSpell(blindeye, SPELL_GREATER_PW_SHIELD))
        return true;
    
    return false;
}