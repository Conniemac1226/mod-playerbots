#ifndef _PLAYERBOT_SLAVEPENSTRIGGERS_H
#define _PLAYERBOT_SLAVEPENSTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"

// Mennu the Betrayer Triggers
class MennuTotemActiveTrigger : public Trigger
{
public:
    MennuTotemActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "mennu totem active") {}
    bool IsActive() override;
};

class MennuLightningBoltCastTrigger : public Trigger
{
public:
    MennuLightningBoltCastTrigger(PlayerbotAI* ai) : Trigger(ai, "mennu lightning bolt cast") {}
    bool IsActive() override;
};

class MennuNovaTotemNearbyTrigger : public Trigger
{
public:
    MennuNovaTotemNearbyTrigger(PlayerbotAI* ai) : Trigger(ai, "nova totem nearby") {}
    bool IsActive() override;
};

// Rokmar the Crackler Triggers
class RokmarEnsnaringMossTrigger : public Trigger
{
public:
    RokmarEnsnaringMossTrigger(PlayerbotAI* ai) : Trigger(ai, "ensnaring moss active") {}
    bool IsActive() override;
};

class RokmarGrievousWoundTrigger : public Trigger
{
public:
    RokmarGrievousWoundTrigger(PlayerbotAI* ai) : Trigger(ai, "grievous wound active") {}
    bool IsActive() override;
};

class RokmarWaterSpitTrigger : public Trigger
{
public:
    RokmarWaterSpitTrigger(PlayerbotAI* ai) : Trigger(ai, "water spit incoming") {}
    bool IsActive() override;
};

// Quagmirran Triggers
class QuagmirranAcidSprayTrigger : public Trigger
{
public:
    QuagmirranAcidSprayTrigger(PlayerbotAI* ai) : Trigger(ai, "acid spray casting") {}
    bool IsActive() override;
};

class QuagmirranPoisonBoltVolleyTrigger : public Trigger
{
public:
    QuagmirranPoisonBoltVolleyTrigger(PlayerbotAI* ai) : Trigger(ai, "poison bolt volley casting") {}
    bool IsActive() override;
};

class QuagmirranUppercutTrigger : public Trigger
{
public:
    QuagmirranUppercutTrigger(PlayerbotAI* ai) : Trigger(ai, "uppercut positioning") {}
    bool IsActive() override;
};

class QuagmirranCleavePositionTrigger : public Trigger
{
public:
    QuagmirranCleavePositionTrigger(PlayerbotAI* ai) : Trigger(ai, "quagmirran cleave positioning") {}
    bool IsActive() override;
};

#endif