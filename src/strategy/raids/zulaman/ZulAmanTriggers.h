#ifndef _PLAYERBOT_RAIDZATRIGGERS_H
#define _PLAYERBOT_RAIDZATRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAI.h"

// Nalorakk (Bear) Triggers
class NalorakkBrutalSwipeTrigger : public Trigger
{
public:
    NalorakkBrutalSwipeTrigger(PlayerbotAI* ai) : Trigger(ai, "nalorakk brutal swipe") {}
    bool IsActive() override;
};

class NalorakkSurgeTrigger : public Trigger
{
public:
    NalorakkSurgeTrigger(PlayerbotAI* ai) : Trigger(ai, "nalorakk surge") {}
    bool IsActive() override;
};

class NalorakkBearFormTrigger : public Trigger
{
public:
    NalorakkBearFormTrigger(PlayerbotAI* ai) : Trigger(ai, "nalorakk bear form") {}
    bool IsActive() override;
};

// Akil'zon (Eagle) Triggers
class AkilzonElectricalStormTrigger : public Trigger
{
public:
    AkilzonElectricalStormTrigger(PlayerbotAI* ai) : Trigger(ai, "akilzon electrical storm") {}
    bool IsActive() override;
};

class AkilzonStaticDisruptionTrigger : public Trigger
{
public:
    AkilzonStaticDisruptionTrigger(PlayerbotAI* ai) : Trigger(ai, "akilzon static disruption") {}
    bool IsActive() override;
};

class AkilzonSoaringEagleTrigger : public Trigger
{
public:
    AkilzonSoaringEagleTrigger(PlayerbotAI* ai) : Trigger(ai, "akilzon soaring eagle") {}
    bool IsActive() override;
};

// Jan'alai (Dragonhawk) Triggers
class JanalaiFireBombTrigger : public Trigger
{
public:
    JanalaiFireBombTrigger(PlayerbotAI* ai) : Trigger(ai, "janalai fire bomb") {}
    bool IsActive() override;
};

class JanalaiHatcherTrigger : public Trigger
{
public:
    JanalaiHatcherTrigger(PlayerbotAI* ai) : Trigger(ai, "janalai hatcher") {}
    bool IsActive() override;
};

class JanalaiHatchlingTrigger : public Trigger
{
public:
    JanalaiHatchlingTrigger(PlayerbotAI* ai) : Trigger(ai, "janalai hatchling") {}
    bool IsActive() override;
};

class JanalaiFireWallTrigger : public Trigger
{
public:
    JanalaiFireWallTrigger(PlayerbotAI* ai) : Trigger(ai, "janalai fire wall") {}
    bool IsActive() override;
};

// Halazzi (Lynx) Triggers
class HalazziLynxTrigger : public Trigger
{
public:
    HalazziLynxTrigger(PlayerbotAI* ai) : Trigger(ai, "halazzi lynx") {}
    bool IsActive() override;
};

class HalazziTotemTrigger : public Trigger
{
public:
    HalazziTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "halazzi totem") {}
    bool IsActive() override;
};

class HalazziSaberLashTrigger : public Trigger
{
public:
    HalazziSaberLashTrigger(PlayerbotAI* ai) : Trigger(ai, "halazzi saber lash") {}
    bool IsActive() override;
};

// Hex Lord Malacrass Triggers
class HexLordDrainPowerTrigger : public Trigger
{
public:
    HexLordDrainPowerTrigger(PlayerbotAI* ai) : Trigger(ai, "hex lord drain power") {}
    bool IsActive() override;
};

class HexLordAddTrigger : public Trigger
{
public:
    HexLordAddTrigger(PlayerbotAI* ai) : Trigger(ai, "hex lord add") {}
    bool IsActive() override;
};

class HexLordSpiritBolleyTrigger : public Trigger
{
public:
    HexLordSpiritBolleyTrigger(PlayerbotAI* ai) : Trigger(ai, "hex lord spirit bolley") {}
    bool IsActive() override;
};

// Zul'jin Triggers
class ZuljinGrievousThrowTrigger : public Trigger
{
public:
    ZuljinGrievousThrowTrigger(PlayerbotAI* ai) : Trigger(ai, "zuljin grievous throw") {}
    bool IsActive() override;
};

class ZuljinCreepingParalysisTrigger : public Trigger
{
public:
    ZuljinCreepingParalysisTrigger(PlayerbotAI* ai) : Trigger(ai, "zuljin creeping paralysis") {}
    bool IsActive() override;
};

class ZuljinFeatherVortexTrigger : public Trigger
{
public:
    ZuljinFeatherVortexTrigger(PlayerbotAI* ai) : Trigger(ai, "zuljin feather vortex") {}
    bool IsActive() override;
};

class ZuljinCycloneTrigger : public Trigger
{
public:
    ZuljinCycloneTrigger(PlayerbotAI* ai) : Trigger(ai, "zuljin cyclone") {}
    bool IsActive() override;
};

class ZuljinFlameColumnTrigger : public Trigger
{
public:
    ZuljinFlameColumnTrigger(PlayerbotAI* ai) : Trigger(ai, "zuljin flame column") {}
    bool IsActive() override;
};

#endif