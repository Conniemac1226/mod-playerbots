#pragma once

#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"

// Boss IDs from AzerothCore
#define NPC_SELIN_FIREHEART     24723
#define NPC_FEL_CRYSTAL         24722
#define NPC_VEXALLUS            24744
#define NPC_PURE_ENERGY         24745
#define NPC_DELRISSA            24560
#define NPC_KAELTHAS            24664
#define NPC_PHOENIX             24674
#define NPC_PHOENIX_EGG         24675
#define NPC_ARCANE_SPHERE       24708
#define NPC_FLAMESTRIKE_TRIGGER 24666

// Spell IDs from AzerothCore
#define SPELL_DRAIN_LIFE        44294
#define SPELL_FEL_EXPLOSION     44314
#define SPELL_DRAIN_MANA        46153
#define SPELL_MANA_RAGE         44320
#define SPELL_PYROBLAST         36819
#define SPELL_FLAMESTRIKE       44192
#define SPELL_GRAVITY_LAPSE     44224
#define SPELL_GRAVITY_LAPSE_FLY 44227
#define SPELL_GRAVITY_LAPSE_DOT 44226
#define SPELL_SHOCK_BARRIER     46165

// Kael'thas
class KaelthasCastingPyroblastTrigger : public Trigger
{
public:
    KaelthasCastingPyroblastTrigger(PlayerbotAI* ai) : Trigger(ai, "kaelthas casting pyroblast") {}
    bool IsActive() override;
};

class KaelthasCastingGravityLapseTrigger : public Trigger
{
public:
    KaelthasCastingGravityLapseTrigger(PlayerbotAI* ai) : Trigger(ai, "kaelthas casting gravity lapse") {}
    bool IsActive() override;
};

class KaelthasArcaneSphereNearbyTrigger : public Trigger
{
public:
    KaelthasArcaneSphereNearbyTrigger(PlayerbotAI* ai) : Trigger(ai, "kaelthas arcane sphere nearby") {}
    bool IsActive() override;
};

class KaelthasMTFlamestrikeTrigger : public Trigger
{
public:
    KaelthasMTFlamestrikeTrigger(PlayerbotAI* ai) : Trigger(ai, "kaelthas flamestrike") {}
    bool IsActive() override;
};

class KaelthasPhoenixesAndEggsTrigger : public Trigger
{
public:
    KaelthasPhoenixesAndEggsTrigger(PlayerbotAI* ai) : Trigger(ai, "kaelthas phoenixes and eggs active") {}
    bool IsActive() override;
};

// Vexallus
class VexallusPureEnergySpawnedTrigger : public Trigger
{
public:
    VexallusPureEnergySpawnedTrigger(PlayerbotAI* ai) : Trigger(ai, "vexallus pure energy spawned") {}
    bool IsActive() override;
};

class VexallusSpreadOutTrigger : public Trigger
{
public:
    VexallusSpreadOutTrigger(PlayerbotAI* ai) : Trigger(ai, "vexallus spread out") {}
    bool IsActive() override;
};

// Selin Fireheart
class SelinFireheartFelExplosionTrigger : public Trigger
{
public:
    SelinFireheartFelExplosionTrigger(PlayerbotAI* ai) : Trigger(ai, "selin fireheart fel explosion") {}
    bool IsActive() override;
};

class SelinFireheartChannelingTrigger : public Trigger
{
public:
    SelinFireheartChannelingTrigger(PlayerbotAI* ai) : Trigger(ai, "selin fireheart channeling") {}
    bool IsActive() override;
};

class FelCrystalNearbyTrigger : public Trigger
{
public:
    FelCrystalNearbyTrigger(PlayerbotAI* ai) : Trigger(ai, "fel crystal nearby") {}
    bool IsActive() override;
};

// Delrissa
class DelrissaAddActiveTrigger : public Trigger
{
public:
    DelrissaAddActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "delrissa add active") {}
    bool IsActive() override;
};
