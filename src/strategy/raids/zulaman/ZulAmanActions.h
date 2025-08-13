#ifndef _PLAYERBOT_RAIDZAACTIONS_H
#define _PLAYERBOT_RAIDZAACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"

// Nalorakk (Bear) Actions
class NalorakkBrutalSwipeAvoidAction : public MovementAction
{
public:
    NalorakkBrutalSwipeAvoidAction(PlayerbotAI* ai) : MovementAction(ai, "nalorakk brutal swipe avoid") {}
    bool Execute(Event event) override;
};

class NalorakkSurgePositionAction : public MovementAction
{
public:
    NalorakkSurgePositionAction(PlayerbotAI* ai) : MovementAction(ai, "nalorakk surge position") {}
    bool Execute(Event event) override;
};

class NalorakkBearFormPositionAction : public MovementAction
{
public:
    NalorakkBearFormPositionAction(PlayerbotAI* ai) : MovementAction(ai, "nalorakk bear form position") {}
    bool Execute(Event event) override;
};

// Akil'zon (Eagle) Actions
class AkilzonElectricalStormAction : public MovementAction
{
public:
    AkilzonElectricalStormAction(PlayerbotAI* ai) : MovementAction(ai, "akilzon electrical storm") {}
    bool Execute(Event event) override;
};

class AkilzonStaticDisruptionAvoidAction : public MovementAction
{
public:
    AkilzonStaticDisruptionAvoidAction(PlayerbotAI* ai) : MovementAction(ai, "akilzon static disruption avoid") {}
    bool Execute(Event event) override;
};

class AkilzonSoaringEagleTargetAction : public AttackAction
{
public:
    AkilzonSoaringEagleTargetAction(PlayerbotAI* ai) : AttackAction(ai, "akilzon eagle target") {}
    bool Execute(Event event) override;
};

// Jan'alai (Dragonhawk) Actions
class JanalaiFireBombAvoidAction : public MovementAction
{
public:
    JanalaiFireBombAvoidAction(PlayerbotAI* ai) : MovementAction(ai, "janalai fire bomb avoid") {}
    bool Execute(Event event) override;
};

class JanalaiHatcherTargetAction : public AttackAction
{
public:
    JanalaiHatcherTargetAction(PlayerbotAI* ai) : AttackAction(ai, "janalai hatcher target") {}
    bool Execute(Event event) override;
};

class JanalaiHatchlingTargetAction : public AttackAction
{
public:
    JanalaiHatchlingTargetAction(PlayerbotAI* ai) : AttackAction(ai, "janalai hatchling target") {}
    bool Execute(Event event) override;
};

class JanalaiFireWallAvoidAction : public MovementAction
{
public:
    JanalaiFireWallAvoidAction(PlayerbotAI* ai) : MovementAction(ai, "janalai fire wall avoid") {}
    bool Execute(Event event) override;
};

// Halazzi (Lynx) Actions
class HalazziLynxTargetAction : public AttackAction
{
public:
    HalazziLynxTargetAction(PlayerbotAI* ai) : AttackAction(ai, "halazzi lynx target") {}
    bool Execute(Event event) override;
};

class HalazziTotemTargetAction : public AttackAction
{
public:
    HalazziTotemTargetAction(PlayerbotAI* ai) : AttackAction(ai, "halazzi totem target") {}
    bool Execute(Event event) override;
};

class HalazziSaberLashPositionAction : public MovementAction
{
public:
    HalazziSaberLashPositionAction(PlayerbotAI* ai) : MovementAction(ai, "halazzi saber lash position") {}
    bool Execute(Event event) override;
};

// Hex Lord Malacrass Actions
class HexLordDrainPowerInterruptAction : public Action
{
public:
    HexLordDrainPowerInterruptAction(PlayerbotAI* ai) : Action(ai, "hex lord drain power interrupt") {}
    bool Execute(Event event) override;
};

class HexLordAddTargetAction : public AttackAction
{
public:
    HexLordAddTargetAction(PlayerbotAI* ai) : AttackAction(ai, "hex lord add target") {}
    bool Execute(Event event) override;
};

class HexLordSpiritBolleyAvoidAction : public MovementAction
{
public:
    HexLordSpiritBolleyAvoidAction(PlayerbotAI* ai) : MovementAction(ai, "hex lord spirit bolley avoid") {}
    bool Execute(Event event) override;
};

// Zul'jin Actions
class ZuljinGrievousThrowHealAction : public Action
{
public:
    ZuljinGrievousThrowHealAction(PlayerbotAI* ai) : Action(ai, "zuljin grievous throw heal") {}
    bool Execute(Event event) override;
};

class ZuljinCreepingParalysisAvoidAction : public MovementAction
{
public:
    ZuljinCreepingParalysisAvoidAction(PlayerbotAI* ai) : MovementAction(ai, "zuljin creeping paralysis avoid") {}
    bool Execute(Event event) override;
};

class ZuljinFeatherVortexTargetAction : public AttackAction
{
public:
    ZuljinFeatherVortexTargetAction(PlayerbotAI* ai) : AttackAction(ai, "zuljin feather vortex target") {}
    bool Execute(Event event) override;
};

class ZuljinCycloneAvoidAction : public MovementAction
{
public:
    ZuljinCycloneAvoidAction(PlayerbotAI* ai) : MovementAction(ai, "zuljin cyclone avoid") {}
    bool Execute(Event event) override;
};

class ZuljinFlameColumnAvoidAction : public MovementAction
{
public:
    ZuljinFlameColumnAvoidAction(PlayerbotAI* ai) : MovementAction(ai, "zuljin flame column avoid") {}
    bool Execute(Event event) override;
};

#endif