#ifndef _PLAYERBOT_GRUULTRIGGERS_H
#define _PLAYERBOT_GRUULTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"
#include "DungeonStrategyUtils.h"
#include "GruulActions.h"

// Gruul the Dragonkiller Triggers
class GruulEngagedTrigger : public Trigger
{
public:
    GruulEngagedTrigger(PlayerbotAI* ai) : Trigger(ai, "gruul engaged") {}
    bool IsActive() override;
};

class GruulGroundSlamTrigger : public Trigger
{
public:
    GruulGroundSlamTrigger(PlayerbotAI* ai) : Trigger(ai, "gruul ground slam") {}
    bool IsActive() override;
};

class GruulShatterTrigger : public Trigger
{
public:
    GruulShatterTrigger(PlayerbotAI* ai) : Trigger(ai, "gruul shatter") {}
    bool IsActive() override;
};

class GruulCaveInTrigger : public Trigger
{
public:
    GruulCaveInTrigger(PlayerbotAI* ai) : Trigger(ai, "gruul cave in") {}
    bool IsActive() override;
};

class GruulReverberationTrigger : public Trigger
{
public:
    GruulReverberationTrigger(PlayerbotAI* ai) : Trigger(ai, "gruul reverberation") {}
    bool IsActive() override;
};

class GruulHurtfulStrikeTrigger : public Trigger
{
public:
    GruulHurtfulStrikeTrigger(PlayerbotAI* ai) : Trigger(ai, "gruul hurtful strike") {}
    bool IsActive() override;
};

class GruulGrowthTrigger : public Trigger
{
public:
    GruulGrowthTrigger(PlayerbotAI* ai) : Trigger(ai, "gruul growth") {}
    bool IsActive() override;
};

// High King Maulgar Triggers
class MaulgarEngagedTrigger : public Trigger
{
public:
    MaulgarEngagedTrigger(PlayerbotAI* ai) : Trigger(ai, "maulgar engaged") {}
    bool IsActive() override;
};

class MaulgarCouncilTrigger : public Trigger
{
public:
    MaulgarCouncilTrigger(PlayerbotAI* ai) : Trigger(ai, "maulgar council") {}
    bool IsActive() override;
};

class MaulgarWhirlwindTrigger : public Trigger
{
public:
    MaulgarWhirlwindTrigger(PlayerbotAI* ai) : Trigger(ai, "maulgar whirlwind") {}
    bool IsActive() override;
};

class MaulgarArcingSmashTrigger : public Trigger
{
public:
    MaulgarArcingSmashTrigger(PlayerbotAI* ai) : Trigger(ai, "maulgar arcing smash") {}
    bool IsActive() override;
};

class MaulgarBerserkerTrigger : public Trigger
{
public:
    MaulgarBerserkerTrigger(PlayerbotAI* ai) : Trigger(ai, "maulgar berserker") {}
    bool IsActive() override;
};

// Council Member Triggers
class KroshSpellshieldTrigger : public Trigger
{
public:
    KroshSpellshieldTrigger(PlayerbotAI* ai) : Trigger(ai, "krosh spellshield") {}
    bool IsActive() override;
};

class KroshBlastWaveTrigger : public Trigger
{
public:
    KroshBlastWaveTrigger(PlayerbotAI* ai) : Trigger(ai, "krosh blast wave") {}
    bool IsActive() override;
};

class KigglerPolymorphTrigger : public Trigger
{
public:
    KigglerPolymorphTrigger(PlayerbotAI* ai) : Trigger(ai, "kiggler polymorph") {}
    bool IsActive() override;
};

class KigglerArcaneExplosionTrigger : public Trigger
{
public:
    KigglerArcaneExplosionTrigger(PlayerbotAI* ai) : Trigger(ai, "kiggler arcane explosion") {}
    bool IsActive() override;
};

class OlmSummonTrigger : public Trigger
{
public:
    OlmSummonTrigger(PlayerbotAI* ai) : Trigger(ai, "olm summon") {}
    bool IsActive() override;
};

class BlindeyeHealTrigger : public Trigger
{
public:
    BlindeyeHealTrigger(PlayerbotAI* ai) : Trigger(ai, "blindeye heal") {}
    bool IsActive() override;
};

class BlindeyeShieldTrigger : public Trigger
{
public:
    BlindeyeShieldTrigger(PlayerbotAI* ai) : Trigger(ai, "blindeye shield") {}
    bool IsActive() override;
};

#endif