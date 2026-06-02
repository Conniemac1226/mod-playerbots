#ifndef _PLAYERBOT_BLACKMORASSACTIONS_H
#define _PLAYERBOT_BLACKMORASSACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// RESEARCHED FROM: src/server/scripts/Kalimdor/CavernsOfTime/TheBlackMorass/the_black_morass.h
enum BlackMorassNpcs
{
    NPC_MEDIVH                  = 15608,
    NPC_TIME_RIFT              = 17838,
    NPC_CHRONO_LORD_DEJA       = 17879,
    NPC_TEMPORUS               = 17880,
    NPC_AEONUS                 = 17881,
    // Portal adds
    NPC_RIFT_KEEPER_WARLOCK    = 21104,
    NPC_RIFT_KEEPER_MAGE       = 21148,
    NPC_RIFT_LORD              = 17839,
    NPC_RIFT_LORD_2            = 21140,
    NPC_INFINITE_ASSASSIN      = 17835,
    NPC_INFINITE_WHELP         = 21818,
    NPC_INFINITE_CHRONOMANCER  = 17892,
    NPC_INFINITE_EXECUTIONER   = 18994,
    NPC_INFINITE_VANQUISHER    = 18995,
    NPC_INFINITE_ASSASSIN_2    = 21137,
    NPC_INFINITE_CHRONOMANCER_2 = 21136,
    NPC_INFINITE_EXECUTIONER_2 = 21138,
    NPC_INFINITE_VANQUISHER_2  = 21139
};

// RESEARCHED FROM: boss scripts
enum BlackMorassSpells
{
    // Aeonus
    SPELL_CLEAVE                = 40504,
    SPELL_TIME_STOP             = 31422,
    SPELL_ENRAGE                = 37605,
    SPELL_SAND_BREATH           = 31473,
    
    // Chrono Lord Deja
    SPELL_ARCANE_BLAST          = 31457,
    SPELL_ARCANE_DISCHARGE      = 31472,
    SPELL_TIME_LAPSE            = 31467,
    SPELL_ATTRACTION            = 38540,
    
    // Temporus
    SPELL_HASTEN                = 31458,
    SPELL_MORTAL_WOUND          = 31464,
    SPELL_WING_BUFFET           = 31475,
    SPELL_REFLECT               = 38592
};

inline bool IsBlackMorassPortalAdd(uint32 entry)
{
    switch (entry)
    {
        case NPC_RIFT_LORD:
        case NPC_RIFT_LORD_2:
        case NPC_RIFT_KEEPER_WARLOCK:
        case NPC_RIFT_KEEPER_MAGE:
        case NPC_INFINITE_EXECUTIONER:
        case NPC_INFINITE_EXECUTIONER_2:
        case NPC_INFINITE_VANQUISHER:
        case NPC_INFINITE_VANQUISHER_2:
        case NPC_INFINITE_CHRONOMANCER:
        case NPC_INFINITE_CHRONOMANCER_2:
        case NPC_INFINITE_ASSASSIN:
        case NPC_INFINITE_ASSASSIN_2:
        case NPC_INFINITE_WHELP:
            return true;
        default:
            return false;
    }
}

// Portal/Add Management Actions
class AttackPortalAddAction : public AttackAction
{
public:
    AttackPortalAddAction(PlayerbotAI* ai) : AttackAction(ai, "attack portal add") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class ProtectMedivhAction : public MovementAction
{
public:
    ProtectMedivhAction(PlayerbotAI* ai) : MovementAction(ai, "protect medivh") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Aeonus Actions
class AeonusAvoidCleaveAction : public MovementAction
{
public:
    AeonusAvoidCleaveAction(PlayerbotAI* ai) : MovementAction(ai, "aeonus avoid cleave") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AeonusPositionAction : public MovementAction
{
public:
    AeonusPositionAction(PlayerbotAI* ai) : MovementAction(ai, "aeonus position") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AeonusSandBreathAction : public MovementAction
{
public:
    AeonusSandBreathAction(PlayerbotAI* ai) : MovementAction(ai, "avoid sand breath") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Chrono Lord Deja Actions
class AvoidTimeLapseAction : public MovementAction
{
public:
    AvoidTimeLapseAction(PlayerbotAI* ai) : MovementAction(ai, "avoid time lapse") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AvoidArcaneDischargeAction : public MovementAction
{
public:
    AvoidArcaneDischargeAction(PlayerbotAI* ai) : MovementAction(ai, "avoid arcane discharge") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class DejaAttractionAction : public MovementAction
{
public:
    DejaAttractionAction(PlayerbotAI* ai) : MovementAction(ai, "handle attraction") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Temporus Actions
class AvoidWingBuffetAction : public MovementAction
{
public:
    AvoidWingBuffetAction(PlayerbotAI* ai) : MovementAction(ai, "avoid wing buffet") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class TemporusMortalWoundAction : public Action
{
public:
    TemporusMortalWoundAction(PlayerbotAI* ai) : Action(ai, "heal mortal wound") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class TemporusReflectAction : public Action
{
public:
    TemporusReflectAction(PlayerbotAI* ai) : Action(ai, "stop casting reflect") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// RESEARCHED: boss_temporus.cpp:54 - HASTEN every 20s, needs dispel/purge
class TemporusHastenDispelAction : public Action
{
public:
    TemporusHastenDispelAction(PlayerbotAI* ai) : Action(ai, "dispel hasten") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// RESEARCHED: boss_chrono_lord_deja.cpp:58 - ARCANE_BLAST every 20s, interruptible
class DejaArcaneBlastInterruptAction : public Action
{
public:
    DejaArcaneBlastInterruptAction(PlayerbotAI* ai) : Action(ai, "interrupt arcane blast") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
