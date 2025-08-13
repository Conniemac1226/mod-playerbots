#include "BlackTempleTriggers.h"
#include "AiObjectContext.h"
#include "GameObject.h"
#include "Unit.h"
#include "Value.h"
#include "Item.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

#define SPELL_IMPALING_SPINE 39837
#define SPELL_TIDAL_SHIELD 39872
#define ITEM_NAJENTUS_SPINE 32408
#define NPC_NAJENTUS 22887

bool NajentusImpaledNearbyTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "high warlord najentus");
    if (!boss || !boss->IsAlive())
        return false;

    // Check for impaled allies within 15 yards
    Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
    if (!membersValue)
        return false;
        
    GuidVector members = membersValue->Get();
    
    for (auto& guid : members)
    {
        Unit* member = botAI->GetUnit(guid);
        if (!member || member == bot)
            continue;

        if (member->HasAura(SPELL_IMPALING_SPINE))
        {
            float distance = bot->GetDistance(member);
            if (distance < 10.0f)
                return true;
        }
    }

    return false;
}

bool NajentusSpineAvailableTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "high warlord najentus");
    if (!boss || !boss->IsAlive())
        return false;

    // Don't pick up spines if we already have one
    if (bot->HasItemCount(ITEM_NAJENTUS_SPINE, 1))
        return false;

    // Only ranged DPS should pick up spines
    if (botAI->IsTank(bot) || botAI->IsHeal(bot) || botAI->IsMelee(bot))
        return false;

    // Look for spine game objects nearby
    Value<GuidVector>* gosValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects no los");
    if (!gosValue)
        return false;
        
    GuidVector gos = gosValue->Get();
    
    for (auto& guid : gos)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go)
            continue;

        // Check if it's a spine object and within range
        if (go->GetGoType() == GAMEOBJECT_TYPE_GENERIC)
        {
            float distance = bot->GetDistance(go);
            if (distance < 40.0f)
                return true;
        }
    }

    return false;
}

bool NajentusTidalShieldUpTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "high warlord najentus");
    if (!boss || !boss->IsAlive())
        return false;

    // Check if boss has Tidal Shield and we have a spine
    return boss->HasAura(SPELL_TIDAL_SHIELD) && bot->HasItemCount(ITEM_NAJENTUS_SPINE, 1);
}

bool NajentusTidalBurstSoonTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "high warlord najentus");
    if (!boss || !boss->IsAlive())
        return false;

    // Check if Tidal Shield is up
    if (!boss->HasAura(SPELL_TIDAL_SHIELD))
        return false;

    // Check if we need to spread for healers and ranged
    if (!botAI->IsRanged(bot) && !botAI->IsHeal(bot))
        return false;

    // Check spacing with other players
    Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
    if (!membersValue)
        return false;
        
    GuidVector members = membersValue->Get();
    
    for (auto& guid : members)
    {
        Unit* member = botAI->GetUnit(guid);
        if (!member || member == bot)
            continue;

        float distance = bot->GetDistance(member);
        if (distance < 8.0f)
            return true;
    }

    return false;
}

// Supremus
#define SPELL_SNARE_SELF_TRIGGER 41922
#define NPC_SUPREMUS 22898
#define NPC_SUPREMUS_VOLCANO 23085

bool SupremusEngagedTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "supremus");
    return boss && boss->IsAlive() && boss->IsEngaged();
}

bool SupremusKitePhaseTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "supremus");
    if (!boss || !boss->IsAlive())
        return false;

    // Check if boss has snare (kite phase)
    if (!boss->HasAura(SPELL_SNARE_SELF_TRIGGER))
        return false;

    // Check if we're the fixate target
    return boss->GetVictim() == bot;
}

bool SupremusVolcanoNearbyTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "supremus");
    if (!boss || !boss->IsAlive())
        return false;

    // Look for volcano NPCs
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        if (unit->GetEntry() == NPC_SUPREMUS_VOLCANO)
        {
            float distance = bot->GetDistance(unit);
            if (distance < 15.0f)
                return true;
        }
    }

    return false;
}

bool SupremusMoltenFlameNearbyTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "supremus");
    if (!boss || !boss->IsAlive())
        return false;

    // Check for molten flame ground effects
    Value<GuidVector>* gosValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects no los");
    if (!gosValue)
        return false;
        
    GuidVector gos = gosValue->Get();
    
    for (auto& guid : gos)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go)
            continue;

        float distance = bot->GetDistance(go);
        if (distance < 8.0f)
            return true;
    }

    return false;
}

bool SupremusTankPhaseTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "supremus");
    if (!boss || !boss->IsAlive())
        return false;

    // Tank phase = no snare
    return !boss->HasAura(SPELL_SNARE_SELF_TRIGGER) && botAI->IsTank(bot);
}

// Shade of Akama
#define NPC_SHADE_OF_AKAMA_TRIGGER 22841
#define NPC_AKAMA_SHADE_TRIGGER 23191  
#define NPC_ASHTONGUE_CHANNELER_TRIGGER 23421
#define NPC_ASHTONGUE_SORCERER_TRIGGER 23215
#define NPC_ASHTONGUE_DEFENDER_TRIGGER 23216

bool ShadeOfAkamaChannelerActiveTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Check for channelers
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_ASHTONGUE_CHANNELER_TRIGGER)
            return true;
    }

    return false;
}

bool ShadeOfAkamaAddsActiveTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Check for adds
    static const uint32 addTypes[] = {
        NPC_ASHTONGUE_SORCERER_TRIGGER,
        NPC_ASHTONGUE_DEFENDER_TRIGGER,
        23523, // Elemental
        23318, // Rogue
        23524  // Spiritbind
    };

    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        for (int i = 0; i < 5; i++)
        {
            if (unit->GetEntry() == addTypes[i])
                return true;
        }
    }

    return false;
}

bool ShadeOfAkamaAkamaLowHealthTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* akama = AI_VALUE2(Unit*, "find target", "akama");
    if (!akama || !akama->IsAlive())
        return false;

    return akama->GetHealthPct() < 25.0f;
}

bool ShadeOfAkamaPhaseOneTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Phase 1 = channelers are active
    Unit* shade = AI_VALUE2(Unit*, "find target", "shade of akama");
    if (!shade)
        return false;

    // If shade is not attackable yet, we're in phase 1
    return shade->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
}

bool ShadeOfAkamaPhaseTwoTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Phase 2 = shade is released and attackable
    Unit* shade = AI_VALUE2(Unit*, "find target", "shade of akama");
    if (!shade || !shade->IsAlive())
        return false;

    return !shade->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
}

// Teron Gorefiend
bool TeronGorefiendShadowOfDeathTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Check if we have Shadow of Death debuff
    const uint32 SPELL_SHADOW_OF_DEATH = 40251;
    return bot->HasAura(SPELL_SHADOW_OF_DEATH);
}

bool TeronGorefiendGhostFormTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Check if we're in ghost form (have Spiritual Vengeance)
    const uint32 SPELL_SPIRITUAL_VENGEANCE = 40268;
    return bot->HasAura(SPELL_SPIRITUAL_VENGEANCE);
}

bool TeronGorefiendDoomBlossomNearbyTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    const uint32 NPC_DOOM_BLOSSOM = 23123;
    
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_DOOM_BLOSSOM)
        {
            float distance = bot->GetDistance(unit);
            if (distance < 20.0f)
                return true;
        }
    }

    return false;
}

bool TeronGorefiendIncinerateTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Check if we have Incinerate debuff
    const uint32 SPELL_INCINERATE = 40239;
    return bot->HasAura(SPELL_INCINERATE);
}

bool TeronGorefiendCrushingShadowsTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (!boss || !boss->IsAlive())
        return false;

    // Check if boss is casting Crushing Shadows
    const uint32 SPELL_CRUSHING_SHADOWS = 40243;
    return boss->FindCurrentSpellBySpellId(SPELL_CRUSHING_SHADOWS);
}

// Gurtogg Bloodboil
bool GurtoggBloodboilEngagedTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    return boss && boss->IsAlive() && boss->IsInCombat();
}

bool GurtoggFelRageTargetTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Check if we have Fel Rage debuffs
    const uint32 SPELL_FEL_RAGE_TARGET = 40604;
    const uint32 SPELL_FEL_RAGE_2 = 40616;
    const uint32 SPELL_FEL_RAGE_3 = 41625;
    
    return bot->HasAura(SPELL_FEL_RAGE_TARGET) || 
           bot->HasAura(SPELL_FEL_RAGE_2) || 
           bot->HasAura(SPELL_FEL_RAGE_3);
}

bool GurtoggArcingSmashTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!boss || !boss->IsAlive())
        return false;

    // Check if boss is casting Arcing Smash
    const uint32 SPELL_ARCING_SMASH1 = 40457;
    const uint32 SPELL_ARCING_SMASH2 = 40599;
    
    return boss->FindCurrentSpellBySpellId(SPELL_ARCING_SMASH1) ||
           boss->FindCurrentSpellBySpellId(SPELL_ARCING_SMASH2);
}

bool GurtoggFelAcidBreathTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!boss || !boss->IsAlive())
        return false;

    // Check if boss is casting Fel Acid Breath
    const uint32 SPELL_FEL_ACID_BREATH1 = 40508;
    const uint32 SPELL_FEL_ACID_BREATH2 = 40595;
    
    return boss->FindCurrentSpellBySpellId(SPELL_FEL_ACID_BREATH1) ||
           boss->FindCurrentSpellBySpellId(SPELL_FEL_ACID_BREATH2);
}

bool GurtoggFelGeyserNearbyTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    const uint32 NPC_FEL_GEYSER = 23254;
    
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_FEL_GEYSER)
        {
            float distance = bot->GetDistance(unit);
            if (distance < 10.0f)
                return true;
        }
    }

    return false;
}

bool GurtoggAcidicWoundHighTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Only for tanks
    if (!botAI->IsTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!boss || !boss->IsAlive())
        return false;

    const uint32 SPELL_ACIDIC_WOUND = 40484;
    Unit* currentTank = boss->GetVictim();
    
    // Check if current tank needs a swap
    if (currentTank)
    {
        if (Aura* aura = currentTank->GetAura(SPELL_ACIDIC_WOUND))
        {
            return aura->GetStackAmount() >= 3;
        }
    }

    return false;
}

// Reliquary of Souls
bool ReliquaryPhaseOneTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* suffering = AI_VALUE2(Unit*, "find target", "essence of suffering");
    return suffering && suffering->IsAlive();
}

bool ReliquaryPhaseTwoTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    return desire && desire->IsAlive();
}

bool ReliquaryPhaseThreeTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* anger = AI_VALUE2(Unit*, "find target", "essence of anger");
    return anger && anger->IsAlive();
}

bool ReliquaryPhaseTransitionTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Check if any essence is transitioning (no essences active but reliquary is in combat)
    Unit* reliquary = AI_VALUE2(Unit*, "find target", "reliquary of souls");
    if (!reliquary || !reliquary->IsInCombat())
        return false;

    Unit* suffering = AI_VALUE2(Unit*, "find target", "essence of suffering");
    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    Unit* anger = AI_VALUE2(Unit*, "find target", "essence of anger");

    // If reliquary is in combat but no essences are active, we're in transition
    return (!suffering || !suffering->IsAlive()) && 
           (!desire || !desire->IsAlive()) && 
           (!anger || !anger->IsAlive());
}

bool ReliquaryEnslavedSoulActiveTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
        return false;
        
    GuidVector npcs = npcsValue->Get();
    
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == 23469) // Enslaved Soul
        {
            float distance = bot->GetDistance(unit);
            if (distance < 40.0f)
                return true;
        }
    }

    return false;
}

bool ReliquarySoulScreamTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* anger = AI_VALUE2(Unit*, "find target", "essence of anger");
    if (!anger || !anger->IsAlive())
        return false;

    const uint32 SPELL_SOUL_SCREAM = 41545;
    return anger->FindCurrentSpellBySpellId(SPELL_SOUL_SCREAM);
}

bool ReliquarySpiteTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    const uint32 SPELL_SPITE = 41376;
    return bot->HasAura(SPELL_SPITE);
}

// Mother Shahraz
bool MotherShahrazEngagedTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "mother shahraz");
    return boss && boss->IsAlive() && boss->IsInCombat();
}

bool MotherShahrazSaberLashTankTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Only for tanks
    if (!botAI->IsTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "mother shahraz");
    if (!boss || !boss->IsAlive())
        return false;

    // Check if boss is targeting us or another tank
    Unit* currentTarget = boss->GetVictim();
    if (!currentTarget)
        return false;

    // Need to stack if we're a tank and boss is engaged
    Player* targetPlayer = currentTarget->ToPlayer();
    return (targetPlayer && botAI->IsTank(targetPlayer)) || currentTarget == bot;
}

bool MotherShahrazFatalAttractionTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    const uint32 SPELL_FATAL_ATTRACTION_AURA = 41001;
    return bot->HasAura(SPELL_FATAL_ATTRACTION_AURA);
}

bool MotherShahrazBeamTargetTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Check for any of the beam debuffs
    const uint32 beamSpells[] = {40860, 40861, 40859, 40827}; // Sinful, Sinister, Vile, Wicked
    
    for (uint32 spellId : beamSpells)
    {
        if (bot->HasAura(spellId))
            return true;
    }

    return false;
}

bool MotherShahrazPrismaticAuraTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "mother shahraz");
    if (!boss || !boss->IsAlive())
        return false;

    // Check which prismatic aura the boss has
    const uint32 prismaticAuras[] = {
        40880, // Shadow
        40882, // Fire
        40883, // Nature
        40891, // Arcane
        40896, // Frost
        40897  // Holy
    };

    for (uint32 auraId : prismaticAuras)
    {
        if (boss->HasAura(auraId))
            return true;
    }

    return false;
}

// Illidari Council
bool IllidariCouncilEngagedTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Check if any council member is engaged
    const char* councilMembers[] = {
        "gathios the shatterer",
        "high nethermancer zerevor",
        "lady malande",
        "veras darkshadow"
    };

    for (const char* memberName : councilMembers)
    {
        Unit* member = AI_VALUE2(Unit*, "find target", memberName);
        if (member && member->IsAlive() && member->IsInCombat())
            return true;
    }

    return false;
}

bool IllidariCouncilTargetPriorityTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (!currentTarget)
        return false;

    // Check if current target is a council member
    const uint32 councilIds[] = {
        22949,  // Gathios the Shatterer
        22950,  // High Nethermancer Zerevor
        22951,  // Lady Malande
        22952   // Veras Darkshadow
    };

    bool isCouncilMember = false;
    for (uint32 id : councilIds)
    {
        if (currentTarget->GetEntry() == id)
        {
            isCouncilMember = true;
            break;
        }
    }

    if (!isCouncilMember)
        return false;

    // Priority: Malande > Veras > Zerevor > Gathios
    // Need to switch if higher priority target is available
    if (currentTarget->GetEntry() == 22951) // Already on Malande (highest priority)
        return false;

    Unit* malande = AI_VALUE2(Unit*, "find target", "lady malande");
    if (malande && malande->IsAlive() && malande->GetHealthPct() > 20.0f)
        return true;

    if (currentTarget->GetEntry() == 22952) // On Veras (second priority)
        return false;

    Unit* veras = AI_VALUE2(Unit*, "find target", "veras darkshadow");
    if (veras && veras->IsAlive() && !veras->HasAura(41476) && veras->GetHealthPct() > 20.0f) // Not vanished
        return true;

    return false;
}

bool IllidariCouncilSpreadTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Check if we're too close to other players
    Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
    if (!membersValue)
        return false;
        
    GuidVector members = membersValue->Get();
    
    int nearbyCount = 0;
    for (auto& guid : members)
    {
        Unit* member = botAI->GetUnit(guid);
        if (!member || member == bot)
            continue;

        float distance = bot->GetDistance(member);
        if (distance < 8.0f)
        {
            nearbyCount++;
            if (nearbyCount >= 2)
                return true;
        }
    }

    return false;
}

bool IllidariCouncilInterruptMalandeTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Only for players who can interrupt
    // Check if bot has any interrupt ability
    bool canInterrupt = false;
    
    // Check common interrupt spell IDs
    const uint32 SPELL_KICK = 1766;           // Rogue
    const uint32 SPELL_COUNTERSPELL = 2139;   // Mage
    const uint32 SPELL_PUMMEL = 6552;         // Warrior
    const uint32 SPELL_MIND_FREEZE = 47528;   // Death Knight
    const uint32 SPELL_WIND_SHEAR = 57994;    // Shaman
    
    if (bot->HasSpell(SPELL_KICK) || bot->HasSpell(SPELL_COUNTERSPELL) ||
        bot->HasSpell(SPELL_PUMMEL) || bot->HasSpell(SPELL_MIND_FREEZE) ||
        bot->HasSpell(SPELL_WIND_SHEAR))
        canInterrupt = true;
    
    if (!canInterrupt)
        return false;

    Unit* malande = AI_VALUE2(Unit*, "find target", "lady malande");
    if (!malande || !malande->IsAlive())
        return false;

    // Check if Malande is casting Circle of Healing or Empowered Smite
    const uint32 SPELL_CIRCLE_OF_HEALING = 41455;
    const uint32 SPELL_EMPOWERED_SMITE = 41471;

    return malande->FindCurrentSpellBySpellId(SPELL_CIRCLE_OF_HEALING) ||
           malande->FindCurrentSpellBySpellId(SPELL_EMPOWERED_SMITE);
}

bool IllidariCouncilConsecrationTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Check for Consecration ground effect near us
    Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
    if (!gathios || !gathios->IsAlive())
        return false;

    // Check if we're in melee range of Gathios (consecration is around him)
    float distance = bot->GetDistance(gathios);
    return distance < 10.0f;
}

bool IllidariCouncilBlizzardTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Check if we're in a Blizzard area
    const uint32 SPELL_BLIZZARD = 41482;
    
    // Check for Blizzard ground effects
    Value<GuidVector>* gosValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects no los");
    if (!gosValue)
        return false;
        
    GuidVector gos = gosValue->Get();
    
    for (auto& guid : gos)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go)
            continue;

        // Blizzard creates ground effects
        float distance = bot->GetDistance(go);
        if (distance < 8.0f)
            return true;
    }

    return false;
}

bool IllidariCouncilFlamestrikeTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Check if Zerevor is casting Flamestrike
    Unit* zerevor = AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
    if (!zerevor || !zerevor->IsAlive())
        return false;

    const uint32 SPELL_FLAMESTRIKE = 41481;
    
    // Check if casting flamestrike and we're in range
    if (zerevor->FindCurrentSpellBySpellId(SPELL_FLAMESTRIKE))
    {
        // Flamestrike is typically targeted at ranged groups
        float distance = bot->GetDistance(zerevor);
        return distance > 15.0f && distance < 40.0f;
    }

    return false;
}

bool IllidariCouncilPoisonTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Check if we have Deadly Poison from Veras
    const uint32 SPELL_DEADLY_POISON = 41485;
    return bot->HasAura(SPELL_DEADLY_POISON);
}

bool IllidariCouncilDivineWrathTrigger::IsActive()
{
    if (!bot || !botAI)
        return false;

    // Check if Malande is casting Divine Wrath
    Unit* malande = AI_VALUE2(Unit*, "find target", "lady malande");
    if (!malande || !malande->IsAlive())
        return false;

    const uint32 SPELL_DIVINE_WRATH = 41472;
    
    // Divine Wrath is a targeted spell - check if she's casting it
    return malande->FindCurrentSpellBySpellId(SPELL_DIVINE_WRATH);
}