#ifndef _PLAYERBOT_SERPENTSHRINETRIGGERS_H
#define _PLAYERBOT_SERPENTSHRINETRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"

class HydrossMarkOfHydrossTrigger : public Trigger
{
public:
    HydrossMarkOfHydrossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hydross mark of hydross") {}
    bool IsActive() override;
};

class HydrossMarkOfCorruptionTrigger : public Trigger
{
public:
    HydrossMarkOfCorruptionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hydross mark of corruption") {}
    bool IsActive() override;
};

class HydrossWaterTombTrigger : public Trigger
{
public:
    HydrossWaterTombTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hydross water tomb") {}
    bool IsActive() override;
};

class HydrossVileSludgeTrigger : public Trigger
{
public:
    HydrossVileSludgeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hydross vile sludge") {}
    bool IsActive() override;
};

class HydrossAddsTrigger : public Trigger
{
public:
    HydrossAddsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hydross adds") {}
    bool IsActive() override;
};

class HydrossTransitionNeededTrigger : public Trigger
{
public:
    HydrossTransitionNeededTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hydross transition needed") {}
    bool IsActive() override;
};

class HydrossTankPositionTrigger : public Trigger
{
public:
    HydrossTankPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hydross tank position") {}
    bool IsActive() override;
};

// The Lurker Below Triggers
class LurkerSpoutTrigger : public Trigger
{
public:
    LurkerSpoutTrigger(PlayerbotAI* botAI) : Trigger(botAI, "lurker spout") {}
    bool IsActive() override;
};

class LurkerWhirlTrigger : public Trigger
{
public:
    LurkerWhirlTrigger(PlayerbotAI* botAI) : Trigger(botAI, "lurker whirl") {}
    bool IsActive() override;
};

class LurkerGeyserTrigger : public Trigger
{
public:
    LurkerGeyserTrigger(PlayerbotAI* botAI) : Trigger(botAI, "lurker geyser") {}
    bool IsActive() override;
};

class LurkerAddsTrigger : public Trigger
{
public:
    LurkerAddsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "lurker adds") {}
    bool IsActive() override;
};

class LurkerPositionTrigger : public Trigger
{
public:
    LurkerPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "lurker position") {}
    bool IsActive() override;
};

// Leotheras the Blind Triggers
class LeotherasWhirlwindTrigger : public Trigger
{
public:
    LeotherasWhirlwindTrigger(PlayerbotAI* botAI) : Trigger(botAI, "leotheras whirlwind") {}
    bool IsActive() override;
};

class LeotherasChaosBlastTrigger : public Trigger
{
public:
    LeotherasChaosBlastTrigger(PlayerbotAI* botAI) : Trigger(botAI, "leotheras chaos blast") {}
    bool IsActive() override;
};

class LeotherasInnerDemonTrigger : public Trigger
{
public:
    LeotherasInnerDemonTrigger(PlayerbotAI* botAI) : Trigger(botAI, "leotheras inner demon") {}
    bool IsActive() override;
};

class LeotherasShadowTrigger : public Trigger
{
public:
    LeotherasShadowTrigger(PlayerbotAI* botAI) : Trigger(botAI, "leotheras shadow") {}
    bool IsActive() override;
};

class LeotherasPositionTrigger : public Trigger
{
public:
    LeotherasPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "leotheras position") {}
    bool IsActive() override;
};

// Fathom-Lord Karathress Triggers
class KarathressCataclysmicBoltTrigger : public Trigger
{
public:
    KarathressCataclysmicBoltTrigger(PlayerbotAI* botAI) : Trigger(botAI, "karathress cataclysmic bolt") {}
    bool IsActive() override;
};

class KarathressSearNovaTrigger : public Trigger
{
public:
    KarathressSearNovaTrigger(PlayerbotAI* botAI) : Trigger(botAI, "karathress sear nova") {}
    bool IsActive() override;
};

class KarathressAdvisorsTrigger : public Trigger
{
public:
    KarathressAdvisorsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "karathress advisors") {}
    bool IsActive() override;
};

class KarathressCycloneTrigger : public Trigger
{
public:
    KarathressCycloneTrigger(PlayerbotAI* botAI) : Trigger(botAI, "karathress cyclone") {}
    bool IsActive() override;
};

class KarathressSpreadTrigger : public Trigger
{
public:
    KarathressSpreadTrigger(PlayerbotAI* botAI) : Trigger(botAI, "karathress spread") {}
    bool IsActive() override;
};

class KarathressClearSpreadTrigger : public Trigger
{
public:
    KarathressClearSpreadTrigger(PlayerbotAI* botAI) : Trigger(botAI, "karathress clear spread") {}
    bool IsActive() override;
};


class KarathressTidalSurgeTrigger : public Trigger
{
public:
    KarathressTidalSurgeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "karathress tidal surge") {}
    bool IsActive() override;
};

class KarathressTotemsTrigger : public Trigger
{
public:
    KarathressTotemsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "karathress totems") {}
    bool IsActive() override;
};

// Morogrim Tidewalker Triggers
class MorogrimTidalWaveTrigger : public Trigger
{
public:
    MorogrimTidalWaveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "morogrim tidal wave") {}
    bool IsActive() override;
};

class MorogrimWateryGraveTrigger : public Trigger
{
public:
    MorogrimWateryGraveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "morogrim watery grave") {}
    bool IsActive() override;
};

class MorogrimMurlocsTrigger : public Trigger
{
public:
    MorogrimMurlocsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "morogrim murlocs") {}
    bool IsActive() override;
};

class MorogrimOfftankMurlocsTrigger : public Trigger
{
public:
    MorogrimOfftankMurlocsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "morogrim offtank murlocs") {}
    bool IsActive() override;
};

class MorogrimGlobulesTrigger : public Trigger
{
public:
    MorogrimGlobulesTrigger(PlayerbotAI* botAI) : Trigger(botAI, "morogrim globules") {}
    bool IsActive() override;
};

class MorogrimPositionTrigger : public Trigger
{
public:
    MorogrimPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "morogrim position") {}
    bool IsActive() override;
};

// Lady Vashj Triggers
class VashjShockBlastTrigger : public Trigger
{
public:
    VashjShockBlastTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vashj shock blast") {}
    bool IsActive() override;
};

class VashjStaticChargeTrigger : public Trigger
{
public:
    VashjStaticChargeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vashj static charge") {}
    bool IsActive() override;
};

class VashjEntangleTrigger : public Trigger
{
public:
    VashjEntangleTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vashj entangle") {}
    bool IsActive() override;
};

class VashjEnchantedElementalTrigger : public Trigger
{
public:
    VashjEnchantedElementalTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vashj enchanted elemental") {}
    bool IsActive() override;
};

class VashjTaintedElementalTrigger : public Trigger
{
public:
    VashjTaintedElementalTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vashj tainted elemental") {}
    bool IsActive() override;
};

class VashjCoilfangEliteTrigger : public Trigger
{
public:
    VashjCoilfangEliteTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vashj coilfang elite") {}
    bool IsActive() override;
};

class VashjCoilfangStriderTrigger : public Trigger
{
public:
    VashjCoilfangStriderTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vashj coilfang strider") {}
    bool IsActive() override;
};

class VashjSporebatTrigger : public Trigger
{
public:
    VashjSporebatTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vashj sporebat") {}
    bool IsActive() override;
};

class VashjPositionTrigger : public Trigger
{
public:
    VashjPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vashj position") {}
    bool IsActive() override;
};

class VashjTaintedCoreTrigger : public Trigger
{
public:
    VashjTaintedCoreTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vashj tainted core") {}
    bool IsActive() override;
};

class VashjMainTankEliteTrigger : public Trigger
{
public:
    VashjMainTankEliteTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vashj main tank elite") {}
    bool IsActive() override;
};

class VashjOfftankAddsTrigger : public Trigger
{
public:
    VashjOfftankAddsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vashj offtank adds") {}
    bool IsActive() override;
};

class VashjForkedLightningTrigger : public Trigger
{
public:
    VashjForkedLightningTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vashj forked lightning") {}
    bool IsActive() override;
};

class VashjElementalOverloadTrigger : public Trigger
{
public:
    VashjElementalOverloadTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vashj elemental overload") {}
    bool IsActive() override;
};

class VashjShieldGeneratorTrigger : public Trigger
{
public:
    VashjShieldGeneratorTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vashj shield generator") {}
    bool IsActive() override;
};

class VashjMultiShotAvoidTrigger : public Trigger
{
public:
    VashjMultiShotAvoidTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vashj multi shot avoid") {}
    bool IsActive() override;
};

class VashjStriderFearTrigger : public Trigger
{
public:
    VashjStriderFearTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vashj strider fear") {}
    bool IsActive() override;
};

#endif
