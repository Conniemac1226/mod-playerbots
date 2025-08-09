#ifndef _PLAYERBOT_BLOODFURNACETRIGGERS_H
#define _PLAYERBOT_BLOODFURNACETRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

enum BloodFurnaceTriggersIds
{
    // The Maker
    THE_MAKER_EXPLODING_BEAKER,
    THE_MAKER_DOMINATION,
    
    // Broggok
    BROGGOK_POISON_CLOUD,
    BROGGOK_INTERRUPT_POISON_BOLT,
    BROGGOK_SLIME_SPRAY,
    
    // Kelidan the Breaker
    KELIDAN_SHADOWMOON_CHANNELER,
    KELIDAN_BURNING_NOVA,
    KELIDAN_INTERRUPT_SHADOW_BOLT_VOLLEY,
    KELIDAN_VORTEX
};

// The Maker Triggers
class TheMakerExplodingBeakerTrigger : public Trigger
{
public:
    TheMakerExplodingBeakerTrigger(PlayerbotAI* ai) : Trigger(ai, "the maker exploding beaker") {}
    bool IsActive() override;
};

class TheMakerDominationTrigger : public Trigger
{
public:
    TheMakerDominationTrigger(PlayerbotAI* ai) : Trigger(ai, "the maker domination") {}
    bool IsActive() override;
};

// Broggok Triggers
class BroggokPoisonCloudTrigger : public Trigger
{
public:
    BroggokPoisonCloudTrigger(PlayerbotAI* ai) : Trigger(ai, "broggok poison cloud") {}
    bool IsActive() override;
};

class BroggokInterruptPoisonBoltTrigger : public Trigger
{
public:
    BroggokInterruptPoisonBoltTrigger(PlayerbotAI* ai) : Trigger(ai, "broggok interrupt poison bolt") {}
    bool IsActive() override;
};

class BroggokSlimeSprayTrigger : public Trigger
{
public:
    BroggokSlimeSprayTrigger(PlayerbotAI* ai) : Trigger(ai, "broggok slime spray") {}
    bool IsActive() override;
};

// Kelidan Triggers
class KelidanShadowmoonChannelerTrigger : public Trigger
{
public:
    KelidanShadowmoonChannelerTrigger(PlayerbotAI* ai) : Trigger(ai, "kelidan shadowmoon channeler") {}
    bool IsActive() override;
};

class KelidanBurningNovaTrigger : public Trigger
{
public:
    KelidanBurningNovaTrigger(PlayerbotAI* ai) : Trigger(ai, "kelidan burning nova") {}
    bool IsActive() override;
};

class KelidanInterruptShadowBoltVolleyTrigger : public Trigger
{
public:
    KelidanInterruptShadowBoltVolleyTrigger(PlayerbotAI* ai) : Trigger(ai, "kelidan interrupt shadow bolt volley") {}
    bool IsActive() override;
};

class KelidanVortexTrigger : public Trigger
{
public:
    KelidanVortexTrigger(PlayerbotAI* ai) : Trigger(ai, "kelidan vortex") {}
    bool IsActive() override;
};

#endif