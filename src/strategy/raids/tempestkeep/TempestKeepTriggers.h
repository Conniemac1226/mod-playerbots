#ifndef _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERS_H
#define _PLAYERBOT_RAIDTEMPESTKEEPTRIGGERS_H

#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Trigger.h"

// Creature IDs following ICC pattern
enum CreatureIdsTempestKeep
{
    // Al'ar
    NPC_ALAR                    = 19514,
    NPC_EMBER_OF_ALAR          = 19551,
    NPC_FLAME_PATCH            = 20602,
    
    // Void Reaver  
    NPC_VOID_REAVER            = 19516,
    
    // High Astromancer Solarian
    NPC_SOLARIAN               = 18805,
    NPC_SOLARIUM_AGENT         = 18925,
    NPC_SOLARIUM_PRIEST        = 18806,
    NPC_ASTROMANCER_LORD       = 20046,
    NPC_ASTROMANCER_SOLARIAN_SPOTLIGHT = 18928,
    
    // Kael'thas Sunstrider
    NPC_KAELTHAS_SUNSTRIDER    = 19622,
    NPC_THALADRED              = 20064,
    NPC_LORD_SANGUINAR         = 20060,
    NPC_GRAND_ASTROMANCER      = 20062,
    NPC_MASTER_ENGINEER        = 20063,
    NPC_TK_PHOENIX             = 21362,
    NPC_TK_PHOENIX_EGG         = 21364,
    NPC_TK_FLAMESTRIKE         = 21369,
    NPC_NETHER_VAPOR           = 21002
};

// Spell IDs following ICC pattern
enum SpellIdsTempestKeep
{
    // Al'ar
    SPELL_FLAME_QUILLS         = 34229,
    SPELL_FLAME_BUFFET         = 34121,
    SPELL_EMBER_BLAST          = 34341,
    SPELL_DIVE_BOMB            = 35181,
    SPELL_DIVE_BOMB_VISUAL     = 35367,
    SPELL_REBIRTH_DIVE         = 35369,
    SPELL_ALAR_CHARGE          = 35412,
    SPELL_MELT_ARMOR           = 35410,
    SPELL_REBIRTH              = 34342,
    
    // Void Reaver
    SPELL_POUNDING             = 34162,
    SPELL_ARCANE_ORB_MISSILE   = 34172,
    SPELL_KNOCK_AWAY           = 25778,
    
    // Solarian
    SPELL_SOLARIAN_ARCANE_MISSILES = 33031,
    SPELL_WRATH_OF_THE_ASTROMANCER = 42783,
    SPELL_SOLARIAN_TRANSFORM   = 39117,
    SPELL_VOID_BOLT            = 39329,
    SPELL_BLINDING_LIGHT       = 33009,
    
    // Kael'thas
    SPELL_KAELTHAS_FIREBALL    = 36805,
    SPELL_PHOENIX              = 36723,
    SPELL_KAELTHAS_FLAMESTRIKE = 36731,
    SPELL_TK_GRAVITY_LAPSE     = 35941,
    SPELL_TK_SHOCK_BARRIER     = 36815,
    SPELL_TK_PYROBLAST         = 36819,
    SPELL_MIND_CONTROL         = 36797,
    SPELL_CONFLAGRATION        = 37018,
    SPELL_REMOTE_TOY           = 37027,
    SPELL_PSYCHIC_BLOW         = 36966,
    SPELL_TK_BELLOWING_ROAR    = 44863
};

// Al'ar triggers
class AlarFlameQuillsTrigger : public Trigger
{
public:
    AlarFlameQuillsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "alar flame quills") {}
    bool IsActive() override;
};

class AlarDiveBombTrigger : public Trigger
{
public:
    AlarDiveBombTrigger(PlayerbotAI* botAI) : Trigger(botAI, "alar dive bomb") {}
    bool IsActive() override;
};

class AlarFlamePatchTrigger : public Trigger  
{
public:
    AlarFlamePatchTrigger(PlayerbotAI* botAI) : Trigger(botAI, "alar flame patch") {}
    bool IsActive() override;
};

class AlarPlatformTrigger : public Trigger
{
public:
    AlarPlatformTrigger(PlayerbotAI* botAI) : Trigger(botAI, "alar platform") {}
    bool IsActive() override;
};

class AlarAddsTrigger : public Trigger
{
public:
    AlarAddsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "alar adds") {}
    bool IsActive() override;
};

// Void Reaver triggers
class VoidReaverPoundingTrigger : public Trigger
{
public:
    VoidReaverPoundingTrigger(PlayerbotAI* botAI) : Trigger(botAI, "void reaver pounding") {}
    bool IsActive() override;
};

class VoidReaverArcaneOrbTrigger : public Trigger
{
public:
    VoidReaverArcaneOrbTrigger(PlayerbotAI* botAI) : Trigger(botAI, "void reaver arcane orb") {}
    bool IsActive() override;
};

class VoidReaverPositionTrigger : public Trigger
{
public:
    VoidReaverPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "void reaver position") {}
    bool IsActive() override;
};

// Solarian triggers
class SolarianWrathTrigger : public Trigger
{
public:
    SolarianWrathTrigger(PlayerbotAI* botAI) : Trigger(botAI, "solarian wrath") {}
    bool IsActive() override;
};

class SolarianBlindingLightTrigger : public Trigger
{
public:
    SolarianBlindingLightTrigger(PlayerbotAI* botAI) : Trigger(botAI, "solarian blinding light") {}
    bool IsActive() override;
};

class SolarianPortalTrigger : public Trigger
{
public:
    SolarianPortalTrigger(PlayerbotAI* botAI) : Trigger(botAI, "solarian portal") {}
    bool IsActive() override;
};

class SolarianAddsTrigger : public Trigger
{
public:
    SolarianAddsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "solarian adds") {}
    bool IsActive() override;
};

// Kael'thas triggers
class KaelthasAdvisorsTrigger : public Trigger
{
public:
    KaelthasAdvisorsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kaelthas advisors") {}
    bool IsActive() override;
};

class KaelthasWeaponsTrigger : public Trigger
{
public:
    KaelthasWeaponsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kaelthas weapons") {}
    bool IsActive() override;
};

class KaelthasPhoenixTrigger : public Trigger
{
public:
    KaelthasPhoenixTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kaelthas phoenix") {}
    bool IsActive() override;
};

class KaelthasFlamestrikeTrigger : public Trigger
{
public:
    KaelthasFlamestrikeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kaelthas flamestrike") {}
    bool IsActive() override;
};

class KaelthasGravityLapseTrigger : public Trigger
{
public:
    KaelthasGravityLapseTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kaelthas gravity lapse") {}
    bool IsActive() override;
};

class KaelthasPyroblastTrigger : public Trigger
{
public:
    KaelthasPyroblastTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kaelthas pyroblast") {}
    bool IsActive() override;
};

class KaelthasMindControlTrigger : public Trigger
{
public:
    KaelthasMindControlTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kaelthas mind control") {}
    bool IsActive() override;
};

class KaelthasNetherVaporTrigger : public Trigger
{
public:
    KaelthasNetherVaporTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kaelthas nether vapor") {}
    bool IsActive() override;
};

class ThaladredFixateTrigger : public Trigger
{
public:
    ThaladredFixateTrigger(PlayerbotAI* botAI) : Trigger(botAI, "thaladred fixate") {}
    bool IsActive() override;
};

class CapernianConflagrationTrigger : public Trigger
{
public:
    CapernianConflagrationTrigger(PlayerbotAI* botAI) : Trigger(botAI, "capernian conflagration") {}
    bool IsActive() override;
};

class TelonicusRemoteToyTrigger : public Trigger
{
public:
    TelonicusRemoteToyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "telonicus remote toy") {}
    bool IsActive() override;
};

#endif