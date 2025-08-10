#ifndef _PLAYERBOT_ARCATRAZACTIONS_H
#define _PLAYERBOT_ARCATRAZACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

const uint32 SPELL_VOID_ZONE = 36119;
const uint32 SPELL_SHADOW_NOVA = 36127;
const uint32 SPELL_SEED_OF_CORRUPTION = 36123;

const uint32 SPELL_WHIRLWIND = 36142;
const uint32 SPELL_GIFT_OF_THE_DOOMSAYER = 36173;
const uint32 SPELL_HEAL = 36144;

const uint32 SPELL_KNOCK_AWAY = 36512;
const uint32 SPELL_CHARGE = 35754;
const uint32 SPELL_FELFIRE = 35769;
const uint32 SPELL_FELFIRE_SHOCK = 35759;

const uint32 SPELL_FEAR = 39415;
const uint32 SPELL_MIND_REND = 36924;
const uint32 SPELL_DOMINATION = 37162;

const uint32 NPC_HARBINGER_ILLUSION = 21466;

class AvoidVoidZoneAction : public MovementAction
{
public:
    AvoidVoidZoneAction(PlayerbotAI* botAI) : MovementAction(botAI, "avoid void zone") {}
    bool Execute(Event event) override;
};

class AvoidShadowNovaAction : public MovementAction
{
public:
    AvoidShadowNovaAction(PlayerbotAI* botAI) : MovementAction(botAI, "avoid shadow nova") {}
    bool Execute(Event event) override;
};

class SeedOfCorruptionDispelAction : public MovementAction
{
public:
    SeedOfCorruptionDispelAction(PlayerbotAI* botAI) : MovementAction(botAI, "seed of corruption dispel") {}
    bool Execute(Event event) override;
};

class DalliahWhirlwindAction : public MovementAction
{
public:
    DalliahWhirlwindAction(PlayerbotAI* botAI) : MovementAction(botAI, "dalliah whirlwind") {}
    bool Execute(Event event) override;
};

class DalliahHealInterruptAction : public Action
{
public:
    DalliahHealInterruptAction(PlayerbotAI* botAI) : Action(botAI, "dalliah heal interrupt") {}
    bool Execute(Event event) override;
};

class SoccothratesKnockAwayAction : public MovementAction
{
public:
    SoccothratesKnockAwayAction(PlayerbotAI* botAI) : MovementAction(botAI, "soccothrates knock away") {}
    bool Execute(Event event) override;
};

class SoccothratesChargeAction : public MovementAction
{
public:
    SoccothratesChargeAction(PlayerbotAI* botAI) : MovementAction(botAI, "soccothrates charge") {}
    bool Execute(Event event) override;
};

class SkyrissIllusionAction : public AttackAction
{
public:
    SkyrissIllusionAction(PlayerbotAI* botAI) : AttackAction(botAI, "skyriss illusion") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

class SkyrissFearAction : public Action
{
public:
    SkyrissFearAction(PlayerbotAI* botAI) : Action(botAI, "skyriss fear") {}
    bool Execute(Event event) override;
};

class SkyrissDominationAction : public Action
{
public:
    SkyrissDominationAction(PlayerbotAI* botAI) : Action(botAI, "skyriss domination") {}
    bool Execute(Event event) override;
};

#endif