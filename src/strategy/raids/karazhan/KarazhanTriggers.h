#ifndef _PLAYERBOT_KARAZHANTRIGGERS_H
#define _PLAYERBOT_KARAZHANTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"
#include "DungeonStrategyUtils.h"
#include "KarazhanActions.h"

// Attumen the Huntsman triggers
class AttumenMountedTrigger : public Trigger
{
public:
    AttumenMountedTrigger(PlayerbotAI* ai) : Trigger(ai, "attumen mounted") {}
    bool IsActive() override;
};

class AttumenChargeDangerTrigger : public Trigger
{
public:
    AttumenChargeDangerTrigger(PlayerbotAI* ai) : Trigger(ai, "attumen charge danger") {}
    bool IsActive() override;
};

class AttumenShadowcleaveTrigger : public Trigger
{
public:
    AttumenShadowcleaveTrigger(PlayerbotAI* ai) : Trigger(ai, "attumen shadowcleave") {}
    bool IsActive() override;
};

// Moroes triggers
class MoroesAddsTrigger : public Trigger
{
public:
    MoroesAddsTrigger(PlayerbotAI* ai) : Trigger(ai, "moroes adds") {}
    bool IsActive() override;
};

class MoroesGarroteTrigger : public Trigger
{
public:
    MoroesGarroteTrigger(PlayerbotAI* ai) : Trigger(ai, "moroes garrote") {}
    bool IsActive() override;
};

// Maiden of Virtue triggers
class MaidenRepentanceTrigger : public Trigger
{
public:
    MaidenRepentanceTrigger(PlayerbotAI* ai) : Trigger(ai, "maiden repentance") {}
    bool IsActive() override;
};

class MaidenHolyGroundTrigger : public Trigger
{
public:
    MaidenHolyGroundTrigger(PlayerbotAI* ai) : Trigger(ai, "maiden holy ground") {}
    bool IsActive() override;
};

// Opera Event triggers
class OperaOzEngagedTrigger : public Trigger
{
public:
    OperaOzEngagedTrigger(PlayerbotAI* ai) : Trigger(ai, "opera oz engaged") {}
    bool IsActive() override;
};

class OperaRomuloJulianneTrigger : public Trigger
{
public:
    OperaRomuloJulianneTrigger(PlayerbotAI* ai) : Trigger(ai, "opera romulo julianne") {}
    bool IsActive() override;
};

class OperaWolfTrigger : public Trigger
{
public:
    OperaWolfTrigger(PlayerbotAI* ai) : Trigger(ai, "opera wolf") {}
    bool IsActive() override;
};

// Curator triggers
class CuratorFlareTrigger : public Trigger
{
public:
    CuratorFlareTrigger(PlayerbotAI* ai) : Trigger(ai, "curator flare") {}
    bool IsActive() override;
};

class CuratorEvocationTrigger : public Trigger
{
public:
    CuratorEvocationTrigger(PlayerbotAI* ai) : Trigger(ai, "curator evocation") {}
    bool IsActive() override;
};

// Shade of Aran triggers
class AranFlameWreathTrigger : public Trigger
{
public:
    AranFlameWreathTrigger(PlayerbotAI* ai) : Trigger(ai, "aran flame wreath") {}
    bool IsActive() override;
};

class AranBlizzardTrigger : public Trigger
{
public:
    AranBlizzardTrigger(PlayerbotAI* ai) : Trigger(ai, "aran blizzard") {}
    bool IsActive() override;
};

class AranDragonsBreathTrigger : public Trigger
{
public:
    AranDragonsBreathTrigger(PlayerbotAI* ai) : Trigger(ai, "aran dragons breath") {}
    bool IsActive() override;
};

class IllhoofDemonChainsTrigger : public Trigger
{
public:
    IllhoofDemonChainsTrigger(PlayerbotAI* ai) : Trigger(ai, "illhoof demon chains") {}
    bool IsActive() override;
};

class IllhoofImpsTrigger : public Trigger
{
public:
    IllhoofImpsTrigger(PlayerbotAI* ai) : Trigger(ai, "illhoof imps") {}
    bool IsActive() override;
};

// Netherspite trigger
class KarazhanNetherspiteTrigger : public Trigger
{
public:
    KarazhanNetherspiteTrigger(PlayerbotAI* ai) : Trigger(ai, "karazhan netherspite") {}
    bool IsActive() override;
};

// Prince Malchezaar triggers
class MalchezaarInfernalTrigger : public Trigger
{
public:
    MalchezaarInfernalTrigger(PlayerbotAI* ai) : Trigger(ai, "malchezaar infernal") {}
    bool IsActive() override;
};

class MalchezaarEnfeebleTrigger : public Trigger
{
public:
    MalchezaarEnfeebleTrigger(PlayerbotAI* ai) : Trigger(ai, "malchezaar enfeeble") {}
    bool IsActive() override;
};

// Nightbane triggers
class NightbaneAirPhaseTrigger : public Trigger
{
public:
    NightbaneAirPhaseTrigger(PlayerbotAI* ai) : Trigger(ai, "nightbane air phase") {}
    bool IsActive() override;
};

class NightbaneCharredEarthTrigger : public Trigger
{
public:
    NightbaneCharredEarthTrigger(PlayerbotAI* ai) : Trigger(ai, "nightbane charred earth") {}
    bool IsActive() override;
};

class NightbaneSkeletonTrigger : public Trigger
{
public:
    NightbaneSkeletonTrigger(PlayerbotAI* ai) : Trigger(ai, "nightbane skeleton") {}
    bool IsActive() override;
};

// Chess Event triggers
class ChessEventActiveTrigger : public Trigger
{
public:
    ChessEventActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "chess event active") {}
    bool IsActive() override;
};

#endif
