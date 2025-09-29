#ifndef _PLAYERBOT_RAIDTEMPESTKEEPMULTIPLIERS_H
#define _PLAYERBOT_RAIDTEMPESTKEEPMULTIPLIERS_H

#include "Multiplier.h"

class AlarFlameQuillsMultiplier : public Multiplier
{
public:
    AlarFlameQuillsMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "alar flame quills") {}
    float GetValue(Action* action) override;
};

class AlarDiveBombMultiplier : public Multiplier
{
public:
    AlarDiveBombMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "alar dive bomb") {}
    float GetValue(Action* action) override;
};

class VoidReaverPoundingMultiplier : public Multiplier
{
public:
    VoidReaverPoundingMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "void reaver pounding") {}
    float GetValue(Action* action) override;
};

class VoidReaverPositionMultiplier : public Multiplier
{
public:
    VoidReaverPositionMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "void reaver position") {}
    float GetValue(Action* action) override;
};

class SolarianWrathMultiplier : public Multiplier
{
public:
    SolarianWrathMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "solarian wrath") {}
    float GetValue(Action* action) override;
};

class SolarianBlindingLightMultiplier : public Multiplier
{
public:
    SolarianBlindingLightMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "solarian blinding light") {}
    float GetValue(Action* action) override;
};

class KaelthasGravityLapseMultiplier : public Multiplier
{
public:
    KaelthasGravityLapseMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "kaelthas gravity lapse") {}
    float GetValue(Action* action) override;
};

class AlarFlameBuffetMultiplier : public Multiplier
{
public:
    AlarFlameBuffetMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "alar flame buffet") {}
    float GetValue(Action* action) override;
};

class AlarEmberBlastMultiplier : public Multiplier
{
public:
    AlarEmberBlastMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "alar ember blast") {}
    float GetValue(Action* action) override;
};

class AlarMeltArmorMultiplier : public Multiplier
{
public:
    AlarMeltArmorMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "alar melt armor") {}
    float GetValue(Action* action) override;
};

class AlarChargeMultiplier : public Multiplier
{
public:
    AlarChargeMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "alar charge") {}
    float GetValue(Action* action) override;
};

#endif