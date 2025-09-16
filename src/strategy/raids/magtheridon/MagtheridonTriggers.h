#ifndef _PLAYERBOT_MAGTHERIDONTRIGGERS_H
#define _PLAYERBOT_MAGTHERIDONTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAI.h"
#include "MagtheridonActions.h"

// Hellfire Channeler needs targeting
class HellfireChannelerNearTrigger : public Trigger
{
public:
    HellfireChannelerNearTrigger(PlayerbotAI* ai) : Trigger(ai, "hellfire channeler near") {}
    bool IsActive() override;
};

// Dark Mending needs interrupt
class DarkMendingCastTrigger : public Trigger
{
public:
    DarkMendingCastTrigger(PlayerbotAI* ai) : Trigger(ai, "dark mending cast") {}
    bool IsActive() override;
};

// Blast Nova incoming
class BlastNovaCastTrigger : public Trigger
{
public:
    BlastNovaCastTrigger(PlayerbotAI* ai) : Trigger(ai, "blast nova cast") {}
    bool IsActive() override;
};

// Need to stop channeling cube
class StopCubeChannelTrigger : public Trigger
{
public:
    StopCubeChannelTrigger(PlayerbotAI* ai) : Trigger(ai, "stop cube channel") {}
    bool IsActive() override;
};

// Quake being cast
class QuakeCastTrigger : public Trigger
{
public:
    QuakeCastTrigger(PlayerbotAI* ai) : Trigger(ai, "quake cast") {}
    bool IsActive() override;
};

// Debris falling
class DebrisFallingTrigger : public Trigger
{
public:
    DebrisFallingTrigger(PlayerbotAI* ai) : Trigger(ai, "debris falling") {}
    bool IsActive() override;
};

// Cave in happening
class CaveInTrigger : public Trigger
{
public:
    CaveInTrigger(PlayerbotAI* ai) : Trigger(ai, "cave in") {}
    bool IsActive() override;
};

// Blaze on ground
class BlazeNearbyTrigger : public Trigger
{
public:
    BlazeNearbyTrigger(PlayerbotAI* ai) : Trigger(ai, "blaze nearby") {}
    bool IsActive() override;
};

// Too close for cleave
class CleaveDangerTrigger : public Trigger
{
public:
    CleaveDangerTrigger(PlayerbotAI* ai) : Trigger(ai, "cleave danger") {}
    bool IsActive() override;
};

// Hellfire Warder active
class HellfireWarderActiveTrigger : public Trigger
{
public:
    HellfireWarderActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "hellfire warder active") {}
    bool IsActive() override;
};

// Tank needs to position adds
class TankPositionAddsTrigger : public Trigger
{
public:
    TankPositionAddsTrigger(PlayerbotAI* ai) : Trigger(ai, "tank position adds") {}
    bool IsActive() override;
};

// Healer needs safe position
class HealerPositionNeededTrigger : public Trigger
{
public:
    HealerPositionNeededTrigger(PlayerbotAI* ai) : Trigger(ai, "healer position needed") {}
    bool IsActive() override;
};

// Banish phase active
class BanishPhaseActiveTrigger : public Trigger
{
public:
    BanishPhaseActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "banish phase active") {}
    bool IsActive() override;
};

// Infernal/Abyssal nearby
class InfernalNearbyTrigger : public Trigger
{
public:
    InfernalNearbyTrigger(PlayerbotAI* ai) : Trigger(ai, "infernal nearby") {}
    bool IsActive() override;
};

// Channeler needs interrupt coordination
class ChannelerInterruptNeededTrigger : public Trigger
{
public:
    ChannelerInterruptNeededTrigger(PlayerbotAI* ai) : Trigger(ai, "channeler interrupt needed") {}
    bool IsActive() override;
};

// Phase transition detected
class MagtheridonPhaseTransitionTrigger : public Trigger
{
public:
    MagtheridonPhaseTransitionTrigger(PlayerbotAI* ai)
        : Trigger(ai, "magtheridon phase transition"), _lastPhase(255)
    {
    }

    bool IsActive() override;

private:
    uint8 _lastPhase;
};

// Shadow Bolt Volley needs interrupt
class ShadowBoltVolleyCastTrigger : public Trigger
{
public:
    ShadowBoltVolleyCastTrigger(PlayerbotAI* ai) : Trigger(ai, "shadow bolt volley cast") {}
    bool IsActive() override;
};

// Mind Exhaustion debuff check
class MindExhaustionCheckTrigger : public Trigger
{
public:
    MindExhaustionCheckTrigger(PlayerbotAI* ai) : Trigger(ai, "mind exhaustion check") {}
    bool IsActive() override;
};

// Magtheridon released
class MagtheridonReleasedTrigger : public Trigger
{
public:
    MagtheridonReleasedTrigger(PlayerbotAI* ai)
        : Trigger(ai, "magtheridon released"), _wasReleased(false)
    {
    }

    bool IsActive() override;

private:
    bool _wasReleased;
};

// Low health phase (30%)
class MagtheridonLowHealthTrigger : public Trigger
{
public:
    MagtheridonLowHealthTrigger(PlayerbotAI* ai)
        : Trigger(ai, "magtheridon low health"), _wasLowHealth(false)
    {
    }

    bool IsActive() override;

private:
    bool _wasLowHealth;
};

#endif

