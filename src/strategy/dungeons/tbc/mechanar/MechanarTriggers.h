#pragma once
#include "Trigger.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "MechanarActions.h"

// ========== MECHANO LORD CAPACITUS TRIGGERS ==========

class CapacitusEngagedTrigger : public Trigger
{
public:
    CapacitusEngagedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "capacitus engaged") {}
    bool IsActive() override;
};

class ReflectiveShieldActiveTrigger : public Trigger
{
public:
    ReflectiveShieldActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "reflective shield active") {}
    bool IsActive() override;
};

class PolarityShiftActiveTrigger : public Trigger
{
public:
    PolarityShiftActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "polarity shift active") {}
    bool IsActive() override;
};

class NetherChargeActiveTrigger : public Trigger
{
public:
    NetherChargeActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "nether charge active") {}
    bool IsActive() override;
};

// ========== NETHERMANCER SEPETHREA TRIGGERS ==========

class SepethreaEngagedTrigger : public Trigger
{
public:
    SepethreaEngagedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sepethrea engaged") {}
    bool IsActive() override;
};

class RagingFlamesActiveTrigger : public Trigger
{
public:
    RagingFlamesActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "raging flames active") {}
    bool IsActive() override;
};

class DragonsBreathDangerTrigger : public Trigger
{
public:
    DragonsBreathDangerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dragons breath danger") {}
    bool IsActive() override;
};

class InfernoDangerTrigger : public Trigger
{
public:
    InfernoDangerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "inferno danger") {}
    bool IsActive() override;
};

// ========== PATHALEON THE CALCULATOR TRIGGERS ==========

class PathaleonEngagedTrigger : public Trigger
{
public:
    PathaleonEngagedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "pathaleon engaged") {}
    bool IsActive() override;
};

class DominationActiveTrigger : public Trigger
{
public:
    DominationActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "domination active") {}
    bool IsActive() override;
};

class NetherWraithActiveTrigger : public Trigger
{
public:
    NetherWraithActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "nether wraith active") {}
    bool IsActive() override;
};

class ArcaneTorrentDangerTrigger : public Trigger
{
public:
    ArcaneTorrentDangerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "arcane torrent danger") {}
    bool IsActive() override;
};

class PathaleonEnragedTrigger : public Trigger
{
public:
    PathaleonEnragedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "pathaleon enraged") {}
    bool IsActive() override;
};

class ArcaneExplosionDangerTrigger : public Trigger
{
public:
    ArcaneExplosionDangerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "arcane explosion danger") {}
    bool IsActive() override;
};

class ManaTapActiveTrigger : public Trigger
{
public:
    ManaTapActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mana tap active") {}
    bool IsActive() override;
};