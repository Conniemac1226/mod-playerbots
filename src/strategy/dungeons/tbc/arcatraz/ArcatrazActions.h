#ifndef _PLAYERBOT_ARCATRAZACTIONS_H
#define _PLAYERBOT_ARCATRAZACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

const uint32 ARC_SPELL_VOID_ZONE = 36119;
const uint32 ARC_SPELL_SHADOW_NOVA = 36127;
const uint32 SPELL_SEED_OF_CORRUPTION = 36123;

const uint32 ARC_SPELL_WHIRLWIND = 36142;
const uint32 SPELL_GIFT_OF_THE_DOOMSAYER = 36173;
const uint32 ARC_SPELL_HEAL = 36144;

const uint32 ARC_SPELL_KNOCK_AWAY = 36512;
const uint32 ARC_SPELL_CHARGE = 35754;
const uint32 SPELL_FELFIRE = 35769;
const uint32 SPELL_FELFIRE_SHOCK = 35759;

// Persistent ground effect NPCs
const uint32 NPC_FELFIRE_GROUND = 20978; // "Wrath-Scryer's Felfire" - persistent ground fire

const uint32 ARC_SPELL_FEAR = 39415;
const uint32 SPELL_MIND_REND = 36924;
const uint32 ARC_SPELL_DOMINATION = 37162;

const uint32 NPC_HARBINGER_ILLUSION = 21466;

// Warden Mellichar's adds
const uint32 NPC_TRICKSTER = 20905;
const uint32 NPC_PH_HUNTER = 20906;
const uint32 NPC_MILLHOUSE = 20977;      // NOTE: Becomes friendly ally, not hostile!
const uint32 NPC_AKKIRIS = 20908;
const uint32 NPC_SULFURON = 20909;
const uint32 NPC_TW_DRAK = 20910;
const uint32 NPC_BL_DRAK = 20911;
const uint32 NPC_HARBINGER_SKYRISS = 20912; // Final boss after adds

class AttackMellicharAddsAction : public AttackAction
{
public:
    AttackMellicharAddsAction(PlayerbotAI* botAI) : AttackAction(botAI, "attack mellichar adds") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class MellicharStopAttackAction : public Action
{
public:
    MellicharStopAttackAction(PlayerbotAI* botAI) : Action(botAI, "mellichar stop attack") {}
    bool Execute(Event event) override;
};

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

class AvoidFelfireGroundAction : public MovementAction
{
public:
    AvoidFelfireGroundAction(PlayerbotAI* botAI) : MovementAction(botAI, "avoid felfire ground") {}
    bool Execute(Event event) override;
};

class SkyrissIllusionAction : public AttackAction
{
public:
    SkyrissIllusionAction(PlayerbotAI* botAI) : AttackAction(botAI, "skyriss illusion") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

class SkyrissFearAction : public MovementAction
{
public:
    SkyrissFearAction(PlayerbotAI* botAI) : MovementAction(botAI, "skyriss fear") {}
    bool Execute(Event event) override;
};

class SkyrissDominationAction : public MovementAction
{
public:
    SkyrissDominationAction(PlayerbotAI* botAI) : MovementAction(botAI, "skyriss domination") {}
    bool Execute(Event event) override;
};

#endif