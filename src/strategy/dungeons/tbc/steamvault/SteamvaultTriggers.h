#pragma once

#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"

class Player;

// Boss IDs from AzerothCore
#define NPC_HYDROMANCER_THESPIA         17797
#define NPC_MEKGINEER_STEAMRIGGER       17796
#define NPC_WARLORD_KALITHRESH          17798
#define NPC_NAGA_DISTILLER              17954
#define NPC_STEAMRIGGER_MECHANIC        17951
#define NPC_THESPIA_WATER_ELEMENTAL     17917

// Spell IDs from AzerothCore
#define SPELL_LIGHTNING_CLOUD           25033
#define SPELL_LUNG_BURST                31481
#define SPELL_ENVELOPING_WINDS          31718
#define SPELL_SUPER_SHRINK_RAY          31485
#define SPELL_SAW_BLADE                 31486
#define SPELL_ELECTRIFIED_NET           35107
#define SPELL_SPELL_REFLECTION          31534
#define SPELL_IMPALE                    39061
#define SPELL_HEAD_CRACK                16172
#define SPELL_WARLORDS_RAGE_DISTILLER   31543

// Hydromancer Thespia
class ThespiaLightningCloudTrigger : public Trigger
{
public:
    ThespiaLightningCloudTrigger(PlayerbotAI* ai) : Trigger(ai, "thespia lightning cloud") {}
    bool IsActive() override;
};

class ThespiaLungBurstTrigger : public Trigger
{
public:
    ThespiaLungBurstTrigger(PlayerbotAI* ai) : Trigger(ai, "thespia lung burst") {}
    bool IsActive() override;
};

class ThespiaWaterElementalActiveTrigger : public Trigger
{
public:
    ThespiaWaterElementalActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "thespia water elemental active") {}
    bool IsActive() override;
};

bool HasAttackableThespiaWaterElemental(PlayerbotAI* botAI, Player* bot);

// Mekgineer Steamrigger
class SteamriggerShrinkRayTrigger : public Trigger
{
public:
    SteamriggerShrinkRayTrigger(PlayerbotAI* ai) : Trigger(ai, "steamrigger shrink ray") {}
    bool IsActive() override;
};

class SteamriggerSawBladeTrigger : public Trigger
{
public:
    SteamriggerSawBladeTrigger(PlayerbotAI* ai) : Trigger(ai, "steamrigger saw blade") {}
    bool IsActive() override;
};

class SteamriggerElectrifiedNetTrigger : public Trigger
{
public:
    SteamriggerElectrifiedNetTrigger(PlayerbotAI* ai) : Trigger(ai, "steamrigger electrified net") {}
    bool IsActive() override;
};

class SteamriggerMechanicActiveTrigger : public Trigger
{
public:
    SteamriggerMechanicActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "steamrigger mechanic active") {}
    bool IsActive() override;
};

bool HasAttackableSteamriggerMechanic(PlayerbotAI* botAI, Player* bot);

// Warlord Kalithresh
class KalithreshSpellReflectionTrigger : public Trigger
{
public:
    KalithreshSpellReflectionTrigger(PlayerbotAI* ai) : Trigger(ai, "kalithresh spell reflection") {}
    bool IsActive() override;
};

class KalithreshImpaleTrigger : public Trigger
{
public:
    KalithreshImpaleTrigger(PlayerbotAI* ai) : Trigger(ai, "kalithresh impale") {}
    bool IsActive() override;
};

bool HasAttackableKalithreshDistiller(PlayerbotAI* botAI, Player* bot);

class KalithreshNagaDistillerActiveTrigger : public Trigger
{
public:
    KalithreshNagaDistillerActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "kalithresh naga distiller active") {}
    bool IsActive() override;
};

class KalithreshChannelingRageTrigger : public Trigger
{
public:
    KalithreshChannelingRageTrigger(PlayerbotAI* ai) : Trigger(ai, "kalithresh channeling rage") {}
    bool IsActive() override;
};

class KalithreshSpellReflectionEndedTrigger : public Trigger
{
public:
    KalithreshSpellReflectionEndedTrigger(PlayerbotAI* ai) : Trigger(ai, "kalithresh spell reflection ended") {}
    bool IsActive() override;
};
