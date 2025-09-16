#ifndef _PLAYERBOT_BOTANICAACTIONS_H
#define _PLAYERBOT_BOTANICAACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

const uint32 SPELL_ARCANE_RESONANCE = 34794;
const uint32 SPELL_ARCANE_DEVASTATION = 34799;
const uint32 SPELL_SUMMON_REINFORCEMENTS = 34803;

const uint32 SPELL_TRANQUILITY = 34550;
const uint32 SPELL_TREE_FORM = 34551;
const uint32 SPELL_SUMMON_FRAYER = 34557;

const uint32 SPELL_ALLERGIC_REACTION = 34697;
const uint32 SPELL_TELEPORT_SELF = 34673;

const uint32 BOT_SPELL_SACRIFICE = 34661;
const uint32 BOT_SPELL_HELLFIRE = 34659;
const uint32 BOT_SPELL_HELLFIRE_HEROIC = 39131;
const uint32 BOT_SPELL_ENRAGE = 34670;

const uint32 BOT_SPELL_WAR_STOMP = 34716;
const uint32 SPELL_ARCANE_VOLLEY = 36705;

const uint32 NPC_FRAYER = 19953;
const uint32 NPC_BLOODWARDER_MENDER = 19633;
const uint32 NPC_BLOODWARDER_RESERVIST = 20078;

class SarannisResonanceDispelAction : public Action
{
public:
    SarannisResonanceDispelAction(PlayerbotAI* botAI) : Action(botAI, "sarannis resonance dispel") {}
    bool Execute(Event event) override;
};

class SarannisReinforcementsAction : public AttackAction
{
public:
    SarannisReinforcementsAction(PlayerbotAI* botAI) : AttackAction(botAI, "sarannis reinforcements") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

class FreywinnFrayerPriorityAction : public AttackAction
{
public:
    FreywinnFrayerPriorityAction(PlayerbotAI* botAI) : AttackAction(botAI, "freywinn frayer priority") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

class FreywinnTranquilityAction : public AttackAction
{
public:
    FreywinnTranquilityAction(PlayerbotAI* botAI) : AttackAction(botAI, "freywinn tranquility") {}
    bool Execute(Event event) override;
};

class LajAllergicReactionAction : public MovementAction
{
public:
    LajAllergicReactionAction(PlayerbotAI* botAI) : MovementAction(botAI, "laj allergic reaction") {}
    bool Execute(Event event) override;
};

class LajTeleportPositionAction : public MovementAction
{
public:
    LajTeleportPositionAction(PlayerbotAI* botAI) : MovementAction(botAI, "laj teleport position") {}
    bool Execute(Event event) override;
};

class ThorngrinSacrificeAction : public MovementAction
{
public:
    ThorngrinSacrificeAction(PlayerbotAI* botAI) : MovementAction(botAI, "thorngrin sacrifice") {}
    bool Execute(Event event) override;
};

class ThorngrinHellfireAction : public MovementAction
{
public:
    ThorngrinHellfireAction(PlayerbotAI* botAI) : MovementAction(botAI, "thorngrin hellfire") {}
    bool Execute(Event event) override;
};

class ThorngrinEnrageAction : public Action
{
public:
    ThorngrinEnrageAction(PlayerbotAI* botAI) : Action(botAI, "thorngrin enrage") {}
    bool Execute(Event event) override;
};

class WarpSplinterWarStompAction : public MovementAction
{
public:
    WarpSplinterWarStompAction(PlayerbotAI* botAI) : MovementAction(botAI, "warp splinter war stomp") {}
    bool Execute(Event event) override;
};

class WarpSplinterArcaneVolleyAction : public MovementAction
{
public:
    WarpSplinterArcaneVolleyAction(PlayerbotAI* botAI) : MovementAction(botAI, "warp splinter arcane volley") {}
    bool Execute(Event event) override;
};

#endif
