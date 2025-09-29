#ifndef _PLAYERBOT_RAIDTEMPESTKEEPACTIONS_H
#define _PLAYERBOT_RAIDTEMPESTKEEPACTIONS_H

#include "Action.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "AttackAction.h"
#include "Value.h"
#include "TempestKeepTriggers.h"

// Position constants following ICC pattern
// Al'ar platform positions from boss script
const Position TK_ALAR_PLATFORM_1 = Position(335.638f, 59.4879f, 17.9319f, 4.60f);
const Position TK_ALAR_PLATFORM_2 = Position(388.751007f, 31.731199f, 20.263599f, 1.61f);
const Position TK_ALAR_PLATFORM_3 = Position(388.790985f, -33.105900f, 20.263599f, 0.52f);
const Position TK_ALAR_PLATFORM_4 = Position(332.722992f, -61.159f, 17.979099f, 5.71f);
const Position TK_ALAR_QUILL_POSITION = Position(332.0f, 0.01f, 43.0f, 0.0f);
const Position TK_ALAR_CENTER_POSITION = Position(331.0f, 0.01f, -2.38f, 0.0f);
const Position TK_ALAR_DIVE_POSITION = Position(332.0f, 0.01f, 43.0f, 0.0f);

// Platform safe spots during quills
const Position TK_ALAR_SAFE_SPOT_1 = Position(340.0f, 58.0f, 17.9f, 0.0f);
const Position TK_ALAR_SAFE_SPOT_2 = Position(390.0f, 35.0f, 20.3f, 0.0f);
const Position TK_ALAR_SAFE_SPOT_3 = Position(390.0f, -35.0f, 20.3f, 0.0f);
const Position TK_ALAR_SAFE_SPOT_4 = Position(335.0f, -58.0f, 18.0f, 0.0f);

// Void Reaver positions
const Position TK_VOID_REAVER_TANK_POSITION = Position(432.59f, -373.73f, 20.0f, 0.0f);
const Position TK_VOID_REAVER_RANGED_POSITION = Position(432.59f, -350.0f, 20.0f, 0.0f);

// Solarian positions
const Position TK_SOLARIAN_TANK_POSITION = Position(432.0f, -373.5f, 17.9f, 0.0f);
const Position TK_SOLARIAN_SPREAD_POSITION = Position(445.0f, -373.5f, 17.9f, 0.0f);

// Kael'thas positions
const Position TK_KAELTHAS_CENTER_POSITION = Position(794.0f, 0.0f, 50.0f, 0.0f);
const Position TK_KAELTHAS_TANK_POSITION = Position(794.0f, 15.0f, 50.0f, 0.0f);

// Helper function for movement (ICC pattern)
class TempestKeepMovementHelper
{
public:
    static bool MoveTowardPosition(Player* bot, const Position& position, float incrementSize);
    static float CalculateDistanceScore(Player* bot, const Position& position);
    static Position CalculateSafePosition(const Position& danger, const Position& current, float minDistance);
};

// Al'ar actions
class AlarFlameQuillsAction : public MovementAction
{
public:
    AlarFlameQuillsAction(PlayerbotAI* botAI) : MovementAction(botAI, "alar flame quills") {}
    bool Execute(Event event) override;
    
private:
    Position GetNearestPlatform();
    bool IsOnPlatform();
};

class AlarDiveBombAction : public MovementAction
{
public:
    AlarDiveBombAction(PlayerbotAI* botAI) : MovementAction(botAI, "alar dive bomb") {}
    bool Execute(Event event) override;
    
private:
    bool IsDiveBombTarget();
    Position CalculateEscapePosition();
};

class AlarFlamePatchAction : public MovementAction
{
public:
    AlarFlamePatchAction(PlayerbotAI* botAI) : MovementAction(botAI, "alar flame patch") {}
    bool Execute(Event event) override;
    
private:
    Unit* FindNearestFlamePatch();
    bool IsInFlamePatch();
};

class AlarPlatformAction : public AttackAction
{
public:
    AlarPlatformAction(PlayerbotAI* botAI) : AttackAction(botAI, "alar platform") {}
    bool Execute(Event event) override;
    
private:
    int GetCurrentPlatform();
    Position GetPlatformPosition(int platform);
    bool ShouldMoveToNextPlatform();
};

class AlarAddsAction : public AttackAction
{
public:
    AlarAddsAction(PlayerbotAI* botAI) : AttackAction(botAI, "alar adds") {}
    bool Execute(Event event) override;

private:
    Unit* FindPriorityAdd();
    void MarkAddWithIcon(Unit* add);
};

class AlarFlameBuffetAction : public AttackAction
{
public:
    AlarFlameBuffetAction(PlayerbotAI* botAI) : AttackAction(botAI, "alar flame buffet") {}
    bool Execute(Event event) override;
};

class AlarEmberBlastAction : public MovementAction
{
public:
    AlarEmberBlastAction(PlayerbotAI* botAI) : MovementAction(botAI, "alar ember blast") {}
    bool Execute(Event event) override;
};

class AlarMeltArmorAction : public AttackAction
{
public:
    AlarMeltArmorAction(PlayerbotAI* botAI) : AttackAction(botAI, "alar melt armor") {}
    bool Execute(Event event) override;
};

class AlarChargeAction : public MovementAction
{
public:
    AlarChargeAction(PlayerbotAI* botAI) : MovementAction(botAI, "alar charge") {}
    bool Execute(Event event) override;
};

// Void Reaver actions
class VoidReaverPoundingAction : public MovementAction
{
public:
    VoidReaverPoundingAction(PlayerbotAI* botAI) : MovementAction(botAI, "void reaver pounding") {}
    bool Execute(Event event) override;
};

class VoidReaverArcaneOrbAction : public MovementAction
{
public:
    VoidReaverArcaneOrbAction(PlayerbotAI* botAI) : MovementAction(botAI, "void reaver arcane orb") {}
    bool Execute(Event event) override;
};

class VoidReaverPositionAction : public AttackAction
{
public:
    VoidReaverPositionAction(PlayerbotAI* botAI) : AttackAction(botAI, "void reaver position") {}
    bool Execute(Event event) override;
    
private:
    bool HandleTankPosition(Unit* boss);
    bool HandleRangedPosition(Unit* boss);
};

// Solarian actions
class SolarianWrathAction : public MovementAction
{
public:
    SolarianWrathAction(PlayerbotAI* botAI) : MovementAction(botAI, "solarian wrath") {}
    bool Execute(Event event) override;
};

class SolarianBlindingLightAction : public MovementAction
{
public:
    SolarianBlindingLightAction(PlayerbotAI* botAI) : MovementAction(botAI, "solarian blinding light") {}
    bool Execute(Event event) override;
};

class SolarianPortalAction : public MovementAction
{
public:
    SolarianPortalAction(PlayerbotAI* botAI) : MovementAction(botAI, "solarian portal") {}
    bool Execute(Event event) override;
    
private:
    bool IsPortalActive();
    Position GetPortalPosition();
};

class SolarianAddsAction : public AttackAction
{
public:
    SolarianAddsAction(PlayerbotAI* botAI) : AttackAction(botAI, "solarian adds") {}
    bool Execute(Event event) override;
    
private:
    Unit* GetPriorityAdd();
};

// Kael'thas actions
class KaelthasAdvisorsAction : public AttackAction
{
public:
    KaelthasAdvisorsAction(PlayerbotAI* botAI) : AttackAction(botAI, "kaelthas advisors") {}
    bool Execute(Event event) override;
    
private:
    Unit* GetNextAdvisor();
    int GetAdvisorKillOrder(uint32 entry);
};

class KaelthasWeaponsAction : public AttackAction
{
public:
    KaelthasWeaponsAction(PlayerbotAI* botAI) : AttackAction(botAI, "kaelthas weapons") {}
    bool Execute(Event event) override;
};

class KaelthasPhoenixAction : public AttackAction
{
public:
    KaelthasPhoenixAction(PlayerbotAI* botAI) : AttackAction(botAI, "kaelthas phoenix") {}
    bool Execute(Event event) override;
    
private:
    Unit* FindPhoenix();
    Unit* FindPhoenixEgg();
};

class KaelthsFlamestrikeAction : public MovementAction
{
public:
    KaelthsFlamestrikeAction(PlayerbotAI* botAI) : MovementAction(botAI, "kaelthas flamestrike") {}
    bool Execute(Event event) override;
    
private:
    Unit* FindFlamestrike();
    bool IsInFlamestrike();
};

class KaelthasGravityLapseAction : public MovementAction
{
public:
    KaelthasGravityLapseAction(PlayerbotAI* botAI) : MovementAction(botAI, "kaelthas gravity lapse") {}
    bool Execute(Event event) override;
    
private:
    void HandleGravityMovement();
};

class KaelthasPyroblastAction : public MovementAction
{
public:
    KaelthasPyroblastAction(PlayerbotAI* botAI) : MovementAction(botAI, "kaelthas pyroblast") {}
    bool Execute(Event event) override;
};

class KaelthasMindControlAction : public Action
{
public:
    KaelthasMindControlAction(PlayerbotAI* botAI) : Action(botAI, "kaelthas mind control") {}
    bool Execute(Event event) override;
};

class KaelthasNetherVaporAction : public MovementAction
{
public:
    KaelthasNetherVaporAction(PlayerbotAI* botAI) : MovementAction(botAI, "kaelthas nether vapor") {}
    bool Execute(Event event) override;
    
private:
    Unit* FindNearestNetherVapor();
};

class ThaladredFixateAction : public AttackAction
{
public:
    ThaladredFixateAction(PlayerbotAI* botAI) : AttackAction(botAI, "thaladred fixate") {}
    bool Execute(Event event) override;
};

class CapernianConflagrationAction : public MovementAction
{
public:
    CapernianConflagrationAction(PlayerbotAI* botAI) : MovementAction(botAI, "capernian conflagration") {}
    bool Execute(Event event) override;
};

class TelonicusRemoteToyAction : public Action
{
public:
    TelonicusRemoteToyAction(PlayerbotAI* botAI) : Action(botAI, "telonicus remote toy") {}
    bool Execute(Event event) override;
};

#endif