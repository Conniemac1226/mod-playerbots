#ifndef _PLAYERBOT_SLAVEPENSACTIONS_H
#define _PLAYERBOT_SLAVEPENSACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// RESEARCHED FROM: src/server/scripts/Outland/CoilfangReservoir/SlavePens/the_slave_pens.h
enum SlavePensNpcs
{
    NPC_MENNU_THE_BETRAYER      = 17941,
    NPC_ROKMAR_THE_CRACKLER     = 17991,
    NPC_QUAGMIRRAN              = 17942,
    // Totem IDs - need to find actual spawned totem entries
    NPC_HEALING_WARD            = 20208,  // Healing Ward totem
    NPC_EARTHGRAB_TOTEM         = 18176,  // Earthgrab Totem
    NPC_STONESKIN_TOTEM         = 18177,  // Stoneskin Totem
    NPC_NOVA_TOTEM              = 18179   // Nova Totem
};

inline bool IsMennuAttackableTotemEntry(uint32 entry)
{
    switch (entry)
    {
        case NPC_HEALING_WARD:
        case NPC_EARTHGRAB_TOTEM:
        case NPC_STONESKIN_TOTEM:
            return true;
        default:
            return false;
    }
}

// RESEARCHED FROM: boss_mennu_the_betrayer.cpp:22-28
enum MennuSpells
{
    SP_SPELL_LIGHTNING_BOLT        = 35010,
    SP_SPELL_HEALING_WARD          = 34980,
    SP_SPELL_EARTHGRAB_TOTEM       = 31981,
    SP_SPELL_STONESKIN_TOTEM       = 31985,
    SP_SPELL_NOVA_TOTEM            = 31991
};

// RESEARCHED FROM: boss_rokmar_the_crackler.cpp:22-28
enum RokmarSpells
{
    SP_SPELL_ENSNARING_MOSS        = 31948,
    SP_SPELL_FRENZY                = 34970,
    SP_SPELL_GRIEVOUS_WOUND_N      = 31956,
    SP_SPELL_GRIEVOUS_WOUND_H      = 38801,
    SP_SPELL_WATER_SPIT            = 35008
};

// RESEARCHED FROM: boss_quagmirran.cpp:22-27
enum QuagmirranSpells
{
    SP_SPELL_ACID_SPRAY            = 38153,
    SP_SPELL_CLEAVE                = 40504,
    SP_SPELL_POISON_BOLT_VOLLEY    = 34780,
    SP_SPELL_UPPERCUT              = 32055
};

// Mennu the Betrayer Actions
class AttackMennuTotemAction : public AttackAction
{
public:
    AttackMennuTotemAction(PlayerbotAI* ai) : AttackAction(ai, "attack mennu totem") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class MennuLightningBoltInterruptAction : public Action
{
public:
    MennuLightningBoltInterruptAction(PlayerbotAI* ai) : Action(ai, "interrupt mennu lightning bolt") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class MennuNovaTotemAction : public MovementAction
{
public:
    MennuNovaTotemAction(PlayerbotAI* ai) : MovementAction(ai, "avoid nova totem") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Rokmar the Crackler Actions
class RokmarEnsnaringMossAction : public Action
{
public:
    RokmarEnsnaringMossAction(PlayerbotAI* ai) : Action(ai, "dispel ensnaring moss") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class RokmarFrenzyAction : public Action
{
public:
    RokmarFrenzyAction(PlayerbotAI* ai) : Action(ai, "rokmar frenzy") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class RokmarGrievousWoundAction : public Action
{
public:
    RokmarGrievousWoundAction(PlayerbotAI* ai) : Action(ai, "heal grievous wound") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class RokmarWaterSpitAction : public MovementAction
{
public:
    RokmarWaterSpitAction(PlayerbotAI* ai) : MovementAction(ai, "avoid water spit") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Quagmirran Actions
class QuagmirranAcidSprayAction : public MovementAction
{
public:
    QuagmirranAcidSprayAction(PlayerbotAI* ai) : MovementAction(ai, "avoid acid spray") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class QuagmirranPoisonBoltVolleyAction : public Action
{
public:
    QuagmirranPoisonBoltVolleyAction(PlayerbotAI* ai) : Action(ai, "interrupt poison bolt volley") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class QuagmirranUppercutAction : public MovementAction
{
public:
    QuagmirranUppercutAction(PlayerbotAI* ai) : MovementAction(ai, "tank position uppercut") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class QuagmirranCleavePositionAction : public MovementAction
{
public:
    QuagmirranCleavePositionAction(PlayerbotAI* ai) : MovementAction(ai, "avoid quagmirran cleave") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
