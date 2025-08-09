#pragma once
#include "Action.h"
#include "AttackAction.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "BlackMorassActionsBase.h"

enum BlackMorassIDs
{
    // Boss NPCs
    NPC_CHRONO_LORD_DEJA            = 17879,
    NPC_TEMPORUS                    = 17880, 
    NPC_AEONUS                      = 17881,
    
    // Spell IDs from boss scripts
    SPELL_ARCANE_BLAST              = 31457,
    SPELL_TIME_LAPSE                = 31467,
    SPELL_ARCANE_DISCHARGE          = 31472,
    SPELL_HASTEN                    = 31458,
    SPELL_MORTAL_WOUND              = 31464,
    SPELL_WING_BUFFET               = 31475,
    SPELL_CLEAVE                    = 40504,
    SPELL_SAND_BREATH               = 31473,
    SPELL_TIME_STOP                 = 31422,
    SPELL_ENRAGE                    = 37605,
    SPELL_ATTRACTION                = 38540,
    SPELL_REFLECT                   = 38592
};

class AeonusAvoidCleaveAction : public MovementAction 
{
public:
    AeonusAvoidCleaveAction(PlayerbotAI* botAI) : MovementAction(botAI, "aeonus avoid cleave") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AeonusPositionAction : public MovementAction
{
public:
    AeonusPositionAction(PlayerbotAI* botAI) : MovementAction(botAI, "aeonus position") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AvoidTimeLapseAction : public MovementAction
{
public:
    AvoidTimeLapseAction(PlayerbotAI* botAI) : MovementAction(botAI, "avoid time lapse") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AvoidArcaneDischargeAction : public MovementAction
{
public:
    AvoidArcaneDischargeAction(PlayerbotAI* botAI) : MovementAction(botAI, "avoid arcane discharge") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AvoidWingBuffetAction : public MovementAction
{
public:
    AvoidWingBuffetAction(PlayerbotAI* botAI) : MovementAction(botAI, "avoid wing buffet") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};