#ifndef _PLAYERBOT_MAGTHERIDONACTIONS_H
#define _PLAYERBOT_MAGTHERIDONACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include <map>

enum MagtheridonNPCs
{
    NPC_MAGTHERIDON                 = 17257,
    NPC_HELLFIRE_CHANNELER          = 17256,
    NPC_HELLFIRE_WARDER             = 18829,
    NPC_HELLFIRE_RAID_TRIGGER       = 17376,
    NPC_TARGET_TRIGGER              = 17474
};

enum MagtheridonSpells
{
    SPELL_SHADOW_CAGE               = 30205,
    SPELL_BLAST_NOVA                = 30616,
    SPELL_CLEAVE                    = 30619,
    SPELL_BLAZE                     = 30541,
    SPELL_BLAZE_SUMMON              = 30542,
    SPELL_BERSERK                   = 27680,
    SPELL_SHADOW_GRASP              = 30410,
    SPELL_SHADOW_GRASP_VISUAL       = 30166,
    SPELL_SHADOW_CAGE_STUN          = 30168,
    SPELL_MIND_EXHAUSTION           = 44032,
    SPELL_QUAKE                     = 30657,
    SPELL_QUAKE_KNOCKBACK           = 30571,
    SPELL_COLLAPSE_DAMAGE           = 36449,
    SPELL_CAMERA_SHAKE              = 36455,
    SPELL_DEBRIS_TARGET             = 30629,
    SPELL_DEBRIS_SPAWN              = 30630,
    SPELL_DEBRIS_DAMAGE             = 30631,
    SPELL_DEBRIS_VISUAL             = 30632,
    
    // Hellfire Channeler spells
    SPELL_SHADOW_BOLT_VOLLEY        = 30510,
    SPELL_DARK_MENDING              = 30528,
    SPELL_BURNING_ABYSSAL           = 30511,
    SPELL_SOUL_TRANSFER             = 30531,
    
    // Hellfire Warder spells
    SPELL_SHADOW_WORD_PAIN          = 34441,
    SPELL_DEATH_COIL                = 30500,
    SPELL_SHADOW_BURST              = 30686,
    SPELL_FEAR                      = 39415
};

enum MagtheridonGameObjects
{
    GO_MAGTHERIDON_DOORS            = 183847,
    GO_MANTICRON_CUBE               = 181713
};

// Hellfire Channeler priority targeting
class HellfireChannelerTargetAction : public AttackAction
{
public:
    HellfireChannelerTargetAction(PlayerbotAI* ai) : AttackAction(ai, "hellfire channeler target") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Interrupt Dark Mending
class InterruptDarkMendingAction : public Action
{
public:
    InterruptDarkMendingAction(PlayerbotAI* ai) : Action(ai, "interrupt dark mending") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Blast Nova handling - click cube
class ClickManticronCubeAction : public MovementAction
{
public:
    ClickManticronCubeAction(PlayerbotAI* ai) : MovementAction(ai, "click manticron cube") {}
    bool Execute(Event event) override;
    bool isUseful() override;
    
private:
    bool CanClickCube();
    GameObject* FindNearestCube();
};

// Stop clicking cube when needed
class StopClickingCubeAction : public Action
{
public:
    StopClickingCubeAction(PlayerbotAI* ai) : Action(ai, "stop clicking cube") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Quake avoidance
class AvoidQuakeAction : public MovementAction
{
public:
    AvoidQuakeAction(PlayerbotAI* ai) : MovementAction(ai, "avoid quake") {}
    bool Execute(Event event) override;
    bool isUseful() override;
    
private:
    Position GetSafePosition();
};

// Debris avoidance
class AvoidDebrisAction : public MovementAction
{
public:
    AvoidDebrisAction(PlayerbotAI* ai) : MovementAction(ai, "avoid debris") {}
    bool Execute(Event event) override;
    bool isUseful() override;
    
private:
    bool IsDebrisFalling(Position const& pos);
    Position FindSafeSpot();
};

// Cave in handling (phase 2)
class HandleCaveInAction : public MovementAction
{
public:
    HandleCaveInAction(PlayerbotAI* ai) : MovementAction(ai, "handle cave in") {}
    bool Execute(Event event) override;
    bool isUseful() override;
    
private:
    std::map<ObjectGuid, uint32> g_caveInLastMoveTime;
    std::map<ObjectGuid, bool> g_caveInSafePosition;
};

// Blaze avoidance
class AvoidBlazeAction : public MovementAction
{
public:
    AvoidBlazeAction(PlayerbotAI* ai) : MovementAction(ai, "avoid blaze") {}
    bool Execute(Event event) override;
    bool isUseful() override;
    
private:
    bool IsBlazeNearby();
    Position GetAwayFromBlaze();
};

// Spread for cleave
class SpreadForCleaveAction : public MovementAction
{
public:
    SpreadForCleaveAction(PlayerbotAI* ai) : MovementAction(ai, "spread for cleave") {}
    bool Execute(Event event) override;
    bool isUseful() override;
    
private:
    bool IsTooCloseToTank();
    Position GetSpreadPosition();
};

// Warder priority targeting
class HellfireWarderTargetAction : public AttackAction
{
public:
    HellfireWarderTargetAction(PlayerbotAI* ai) : AttackAction(ai, "hellfire warder target") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Tank positioning for adds
class TankPositionAddsAction : public MovementAction
{
public:
    TankPositionAddsAction(PlayerbotAI* ai) : MovementAction(ai, "tank position adds") {}
    bool Execute(Event event) override;
    bool isUseful() override;
    
private:
    Position GetTankPosition();
};

// Healer positioning
class HealerPositionMagtheridonAction : public MovementAction
{
public:
    HealerPositionMagtheridonAction(PlayerbotAI* ai) : MovementAction(ai, "healer position magtheridon") {}
    bool Execute(Event event) override;
    bool isUseful() override;
    
private:
    Position GetHealerSafeSpot();
};

// Banish phase positioning
class BanishPhasePositionAction : public MovementAction
{
public:
    BanishPhasePositionAction(PlayerbotAI* ai) : MovementAction(ai, "banish phase position") {}
    bool Execute(Event event) override;
    bool isUseful() override;
    
private:
    Position GetBanishPosition();
};

// Infernal/Abyssal avoidance
class AvoidInfernalAction : public MovementAction
{
public:
    AvoidInfernalAction(PlayerbotAI* ai) : MovementAction(ai, "avoid infernal") {}
    bool Execute(Event event) override;
    bool isUseful() override;
    
private:
    bool IsInfernalNearby();
    Position GetSafeFromInfernal();
};

// Channeler interrupt coordination
class CoordinateChannelerInterruptAction : public Action
{
public:
    CoordinateChannelerInterruptAction(PlayerbotAI* ai) : Action(ai, "coordinate channeler interrupt") {}
    bool Execute(Event event) override;
    bool isUseful() override;
    
private:
    Unit* FindChannelerCasting();
    bool ShouldIInterrupt(Unit* channeler);
};

// Phase transition handling
class MagtheridonPhaseTransitionAction : public Action
{
public:
    MagtheridonPhaseTransitionAction(PlayerbotAI* ai) : Action(ai, "magtheridon phase transition") {}
    bool Execute(Event event) override;
    bool isUseful() override;
    
private:
    uint8 GetCurrentPhase();
    void HandlePhaseChange(uint8 newPhase);
};

#endif