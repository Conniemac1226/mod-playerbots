#ifndef _PLAYERBOT_MANATOMBSACTIONS_H
#define _PLAYERBOT_MANATOMBSACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// RESEARCHED FROM: src/server/scripts/Outland/Auchindoun/ManaTombs/mana_tombs.h and boss files
enum ManaTombsNpcs
{
    NPC_PANDEMONIUS         = 18341,  // Standard NPC ID for Pandemonius
    NPC_TAVAROK             = 18343,  // Standard NPC ID for Tavarok
    NPC_NEXUSPRINCE_SHAFFAR = 18344,  // Standard NPC ID for Nexus-Prince Shaffar
    NPC_YOR                 = 22930,  // Standard NPC ID for Yor (summoned boss)
    NPC_ETHEREAL_BEACON     = 18431,  // Shaffar's beacon adds
    // Trash mobs
    NPC_SCAVENGER           = 18309,
    NPC_CRYPT_RAIDER        = 18311,
    NPC_SORCERER            = 18313
};

// RESEARCHED FROM: boss_pandemonius.cpp:31-34
enum PandemoniusSpells
{
    SPELL_VOID_BLAST = 32325,
    SPELL_DARK_SHELL = 32358
};

// RESEARCHED FROM: boss_tavarok.cpp:22-26
enum TavarokSpells
{
    SPELL_EARTHQUAKE        = 33919,
    SPELL_CRYSTAL_PRISON    = 32361,
    SPELL_ARCING_SMASH      = 8374
};

// RESEARCHED FROM: boss_nexusprince_shaffar.cpp:32-44
enum ShaffarSpells
{
    SPELL_BLINK                     = 34605,
    SPELL_FROSTBOLT                 = 32364,
    SPELL_FIREBALL                  = 32363,
    SPELL_FROSTNOVA                 = 32365,
    SPELL_ETHEREAL_BEACON           = 32371,
    SPELL_ETHEREAL_BEACON_VISUAL    = 32368
};

// RESEARCHED FROM: boss_nexusprince_shaffar.cpp:43-44 (Yor spells)
enum YorSpells
{
    SPELL_DOUBLE_BREATH = 38361,
    SPELL_STOMP         = 36405
};

// Pandemonius Actions
class PandemoniusDarkShellAction : public MovementAction
{
public:
    PandemoniusDarkShellAction(PlayerbotAI* ai) : MovementAction(ai, "avoid dark shell") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class PandemoniusVoidBlastAction : public MovementAction
{
public:
    PandemoniusVoidBlastAction(PlayerbotAI* ai) : MovementAction(ai, "spread void blast") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Tavarok Actions
class TavarokEarthquakeAction : public MovementAction
{
public:
    TavarokEarthquakeAction(PlayerbotAI* ai) : MovementAction(ai, "avoid earthquake") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class TavarokCrystalPrisonAction : public AttackAction
{
public:
    TavarokCrystalPrisonAction(PlayerbotAI* ai) : AttackAction(ai, "break crystal prison") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class TavarokArcingSmashAction : public MovementAction
{
public:
    TavarokArcingSmashAction(PlayerbotAI* ai) : MovementAction(ai, "avoid arcing smash") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Nexus-Prince Shaffar Actions
class AttackEtherealBeaconAction : public AttackAction
{
public:
    AttackEtherealBeaconAction(PlayerbotAI* ai) : AttackAction(ai, "attack ethereal beacon") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class ShaffarFrostNovaAction : public MovementAction
{
public:
    ShaffarFrostNovaAction(PlayerbotAI* ai) : MovementAction(ai, "avoid frost nova") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class ShaffarBlinkAction : public AttackAction
{
public:
    ShaffarBlinkAction(PlayerbotAI* ai) : AttackAction(ai, "shaffar blink repositioning") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Yor Actions
class YorDoubleBreathAction : public MovementAction
{
public:
    YorDoubleBreathAction(PlayerbotAI* ai) : MovementAction(ai, "avoid double breath") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class YorStompAction : public MovementAction
{
public:
    YorStompAction(PlayerbotAI* ai) : MovementAction(ai, "avoid stomp") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif