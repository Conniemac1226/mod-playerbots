#include "GruulActions.h"
#include "GruulHelpers.h"
#include "CreatureAI.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "Unit.h"
#include <algorithm>

using namespace GruulsLairHelpers;

namespace
{
    constexpr float MAULGAR_COUNCIL_REPOSITION_DISTANCE = 6.0f;
    constexpr float MAULGAR_COUNCIL_ATTACK_HOLD_RANGE = 35.0f;

    bool ShouldRepositionBeforeCouncilAttack(Player* bot, Unit* target, Position const& safePos)
    {
        if (!bot || !target)
            return false;

        float const distanceToSafePos =
            bot->GetExactDist2d(safePos.GetPositionX(), safePos.GetPositionY());
        if (distanceToSafePos <= MAULGAR_COUNCIL_REPOSITION_DISTANCE)
            return false;

        if (bot->GetVictim() == target &&
            bot->IsWithinDistInMap(target, MAULGAR_COUNCIL_ATTACK_HOLD_RANGE))
            return false;

        return true;
    }
}

// High King Maulgar Actions

// Main tank on Maulgar
bool HighKingMaulgarMainTankAttackMaulgarAction::Execute(Event /*event*/)
{
    Unit* maulgar = FindHighKingMaulgar(botAI, bot);
    if (!maulgar)
        return false;

    MarkTargetWithSquare(bot, maulgar);
    SetRtiTarget(botAI, "square", maulgar);

    if (bot->GetVictim() != maulgar)
        return Attack(maulgar);

    if (maulgar->GetVictim() == bot)
    {
        const Position& position = MAULGAR_TANK_POSITION;
        const float maxDistance = 3.0f;

        float distanceToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distanceToPosition > maxDistance)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveX = bot->GetPositionX() + (dX / distanceToPosition) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / distanceToPosition) * maxDistance;
            return MoveTo(GRUULS_LAIR_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// First offtank on Olm
bool HighKingMaulgarFirstAssistTankAttackOlmAction::Execute(Event /*event*/)
{
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    if (!olm)
        return false;

    MarkTargetWithCircle(bot, olm);
    SetRtiTarget(botAI, "circle", olm);

    if (bot->GetVictim() != olm)
        return Attack(olm);

    if (olm->GetVictim() == bot)
    {
        const Position& position = OLM_TANK_POSITION;
        const float maxDistance = 3.0f;
        const float olmTankLeeway = 30.0f;

        float distanceOlmToPosition = olm->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        float distanceBotToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distanceOlmToPosition > olmTankLeeway && distanceBotToPosition > 0.1f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveX = bot->GetPositionX() + (dX / distanceBotToPosition) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / distanceBotToPosition) * maxDistance;
            return MoveTo(GRUULS_LAIR_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// Second offtank on Blindeye
bool HighKingMaulgarSecondAssistTankAttackBlindeyeAction::Execute(Event /*event*/)
{
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    if (!blindeye)
        return false;

    MarkTargetWithStar(bot, blindeye);
    SetRtiTarget(botAI, "star", blindeye);

    if (bot->GetVictim() != blindeye)
        return Attack(blindeye);

    if (blindeye->GetVictim() == bot)
    {
        const Position& position = BLINDEYE_TANK_POSITION;
        const float maxDistance = 3.0f;

        float distanceToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distanceToPosition > maxDistance)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveX = bot->GetPositionX() + (dX / distanceToPosition) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / distanceToPosition) * maxDistance;
            return MoveTo(GRUULS_LAIR_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// Mage with highest max HP on Krosh
bool HighKingMaulgarMageTankAttackKroshAction::Execute(Event /*event*/)
{
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    if (!krosh)
        return false;

    MarkTargetWithTriangle(bot, krosh);
    SetRtiTarget(botAI, "triangle", krosh);

    if (krosh->HasAura(SPELL_SPELL_SHIELD) && botAI->CanCastSpell("spellsteal", krosh))
        return botAI->CastSpell("spellsteal", krosh);

    if (bot->GetVictim() != krosh)
        return Attack(krosh);

    if (!bot->HasAura(SPELL_SPELL_SHIELD) && botAI->CanCastSpell("fire ward", bot))
        return botAI->CastSpell("fire ward", bot);

    if (krosh->GetVictim() == bot)
    {
        const Position& position = KROSH_TANK_POSITION;
        float distanceToKrosh = krosh->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
        const float minDistance = 16.0f;
        const float maxDistance = 29.0f;
        const float tankPositionLeeway = 1.0f;

        if (distanceToKrosh > minDistance && distanceToKrosh < maxDistance)
        {
            if (!bot->IsWithinDist2d(position.GetPositionX(), position.GetPositionY(), tankPositionLeeway))
            {
                return MoveTo(GRUULS_LAIR_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
                              false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
            }

            float orientation = atan2(krosh->GetPositionY() - bot->GetPositionY(),
                                      krosh->GetPositionX() - bot->GetPositionX());
            bot->SetFacingTo(orientation);
        }
        else
        {
            Position safePos;
            if (TryGetNewSafePosition(botAI, bot, safePos))
            {
                return MoveTo(GRUULS_LAIR_MAP_ID, safePos.GetPositionX(), safePos.GetPositionY(), safePos.GetPositionZ(),
                              false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
    }

    return false;
}

// Moonkin with highest max HP on Kiggler
bool HighKingMaulgarMoonkinTankAttackKigglerAction::Execute(Event /*event*/)
{
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    if (!kiggler)
        return false;

    MarkTargetWithDiamond(bot, kiggler);
    SetRtiTarget(botAI, "diamond", kiggler);

    if (bot->GetVictim() != kiggler)
        return Attack(kiggler);

    if (kiggler->GetVictim() == bot)
    {
        const Position& position = KIGGLER_TANK_POSITION;
        float distanceToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distanceToPosition > 3.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(8.0f, distanceToPosition);
            float moveX = bot->GetPositionX() + (dX / distanceToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distanceToPosition) * moveDist;
            return MoveTo(GRUULS_LAIR_MAP_ID, moveX, moveY, position.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT,
                          true, false);
        }
    }

    return false;
}

bool HighKingMaulgarAssignDPSPriorityAction::Execute(Event /*event*/)
{
    // Target priority 1: Blindeye
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    if (blindeye)
    {
        Position safePos;
        if (botAI->IsRanged(bot) &&
            TryGetMaulgarCouncilPosition(botAI, bot, blindeye, safePos) &&
            ShouldRepositionBeforeCouncilAttack(bot, blindeye, safePos))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);
            return MoveTo(GRUULS_LAIR_MAP_ID, safePos.GetPositionX(), safePos.GetPositionY(), safePos.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        SetRtiTarget(botAI, "star", blindeye);

        if (bot->GetVictim() != blindeye)
            return Attack(blindeye);

        return false;
    }

    // Target priority 2: Olm
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    if (olm)
    {
        Position safePos;
        if (botAI->IsRanged(bot) &&
            TryGetMaulgarCouncilPosition(botAI, bot, olm, safePos) &&
            ShouldRepositionBeforeCouncilAttack(bot, olm, safePos))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);
            return MoveTo(GRUULS_LAIR_MAP_ID, safePos.GetPositionX(), safePos.GetPositionY(), safePos.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        SetRtiTarget(botAI, "circle", olm);

        if (bot->GetVictim() != olm)
            return Attack(olm);

        return false;
    }

    // Target priority 3a: Krosh (ranged only)
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    if (krosh && botAI->IsRanged(bot))
    {
        Position safePos;
        if (TryGetMaulgarCouncilPosition(botAI, bot, krosh, safePos) &&
            ShouldRepositionBeforeCouncilAttack(bot, krosh, safePos))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);
            return MoveTo(GRUULS_LAIR_MAP_ID, safePos.GetPositionX(), safePos.GetPositionY(), safePos.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        SetRtiTarget(botAI, "triangle", krosh);

        if (bot->GetVictim() != krosh)
            return Attack(krosh);

        return false;
    }

    // Target priority 3b: Kiggler
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    if (kiggler && botAI->IsRanged(bot))
    {
        Position safePos;
        if (TryGetMaulgarCouncilPosition(botAI, bot, kiggler, safePos) &&
            ShouldRepositionBeforeCouncilAttack(bot, kiggler, safePos))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);
            return MoveTo(GRUULS_LAIR_MAP_ID, safePos.GetPositionX(), safePos.GetPositionY(), safePos.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        SetRtiTarget(botAI, "diamond", kiggler);

        if (bot->GetVictim() != kiggler)
            return Attack(kiggler);

        return false;
    }

    // Target priority 4: Maulgar
    Unit* maulgar = FindHighKingMaulgar(botAI, bot);
    if (maulgar)
    {
        Position safePos;
        if (botAI->IsRanged(bot) &&
            TryGetMaulgarCouncilPosition(botAI, bot, maulgar, safePos) &&
            ShouldRepositionBeforeCouncilAttack(bot, maulgar, safePos))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);
            return MoveTo(GRUULS_LAIR_MAP_ID, safePos.GetPositionX(), safePos.GetPositionY(), safePos.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        SetRtiTarget(botAI, "square", maulgar);

        if (bot->GetVictim() != maulgar)
            return Attack(maulgar);
    }

    return false;
}

// Avoid Whirlwind and Blast Wave and generally try to stay near the center of the room
bool HighKingMaulgarHealerFindSafePositionAction::Execute(Event /*event*/)
{
    Position safePos;
    if (TryGetMaulgarCouncilPosition(botAI, bot, nullptr, safePos))
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);
        return MoveTo(GRUULS_LAIR_MAP_ID, safePos.GetPositionX(), safePos.GetPositionY(), safePos.GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool HighKingMaulgarInterruptBlindeyeAction::Execute(Event /*event*/)
{
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    if (!blindeye || !blindeye->IsAlive())
        return false;

    if (!blindeye->FindCurrentSpellBySpellId(SPELL_HEAL) &&
        !blindeye->FindCurrentSpellBySpellId(SPELL_PRAYER_OF_HEALING))
        return false;

    if (bot->IsWithinMeleeRange(blindeye))
    {
        botAI->InterruptSpell();
        return true;
    }

    Value<std::list<uint32>>* spellIdsValue =
        botAI->GetAiObjectContext()->GetValue<std::list<uint32>>(
            "spell list", "interrupt");
    if (!spellIdsValue)
        return false;

    std::list<uint32> spellIds = spellIdsValue->Get();
    for (uint32 spellId : spellIds)
    {
        if (botAI->CanCastSpell(spellId, blindeye, false))
            return botAI->CastSpell(spellId, blindeye);
    }

    return false;
}

// Run away from Maulgar during Whirlwind (logic for after all other ogres are dead)
bool HighKingMaulgarRunAwayFromWhirlwindAction::Execute(Event /*event*/)
{
    Unit* maulgar = FindHighKingMaulgar(botAI, bot);
    if (!maulgar)
        return false;

    const float safeDistance = 10.0f;
    float distance = bot->GetExactDist2d(maulgar);

    if (distance < safeDistance)
    {
        float angle = atan2(bot->GetPositionY() - maulgar->GetPositionY(),
                            bot->GetPositionX() - maulgar->GetPositionX());
        float destX = maulgar->GetPositionX() + safeDistance * cos(angle);
        float destY = maulgar->GetPositionY() + safeDistance * sin(angle);
        float destZ = bot->GetPositionZ();

        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
            bot->GetPositionZ(), destX, destY, destZ))
            return false;

        float destDist = maulgar->GetExactDist2d(destX, destY);

        if (destDist >= safeDistance - 0.1f)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveTo(GRUULS_LAIR_MAP_ID, destX, destY, destZ, false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool HighKingMaulgarBanishFelstalkerAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    std::vector<Unit*> felStalkers;
    for (auto const& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == NPC_WILD_FEL_STALKER && unit->IsAlive())
            felStalkers.push_back(unit);
    }
    std::sort(felStalkers.begin(), felStalkers.end(),
              [](Unit* first, Unit* second)
              {
                  return first->GetGUID() < second->GetGUID();
              });

    std::vector<Player*> warlocks;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_WARLOCK && GET_PLAYERBOT_AI(member))
            warlocks.push_back(member);
    }
    std::sort(warlocks.begin(), warlocks.end(),
              [](Player* first, Player* second)
              {
                  return first->GetGUID() < second->GetGUID();
              });

    int warlockIndex = -1;
    for (size_t i = 0; i < warlocks.size(); ++i)
    {
        if (warlocks[i] == bot)
        {
            warlockIndex = static_cast<int>(i);
            break;
        }
    }

    const int64_t felStalkersSize = felStalkers.size();

    if (warlockIndex >= 0 && warlockIndex < felStalkersSize)
    {
        Unit* assignedFelStalker = felStalkers[warlockIndex];
        if (!botAI->HasAura("banish", assignedFelStalker) && botAI->CanCastSpell("banish", assignedFelStalker))
            return botAI->CastSpell("banish", assignedFelStalker);
    }

    return false;
}

// Hunter 1: Misdirect Olm to first offtank and have pet attack Blindeye
// Hunter 2: Misdirect Blindeye to second offtank
bool HighKingMaulgarMisdirectOlmAndBlindeyeAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member))
            hunters.push_back(member);
    }
    std::sort(hunters.begin(), hunters.end(),
              [](Player* first, Player* second)
              {
                  return first->GetGUID() < second->GetGUID();
              });

    int hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int>(i);
            break;
        }
    }

    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    Player* olmTank = nullptr;
    Player* blindeyeTank = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;
        else if (botAI->IsAssistTankOfIndex(member, 0)) olmTank = member;
        else if (botAI->IsAssistTankOfIndex(member, 1)) blindeyeTank = member;
    }

    switch (hunterIndex)
    {
    case 0:
        if (!olm || !blindeye || !olmTank)
            return false;

        botAI->CastSpell("misdirection", olmTank);
        if (bot->HasAura(SPELL_MISDIRECTION))
        {
            Pet* pet = bot->GetPet();
            if (pet && pet->IsAlive() && pet->GetVictim() != blindeye)
            {
                pet->ClearUnitState(UNIT_STATE_FOLLOW);
                pet->AttackStop();
                pet->SetTarget(blindeye->GetGUID());
                if (pet->GetCharmInfo())
                {
                    pet->GetCharmInfo()->SetIsCommandAttack(true);
                    pet->GetCharmInfo()->SetIsAtStay(false);
                    pet->GetCharmInfo()->SetIsFollowing(false);
                    pet->GetCharmInfo()->SetIsCommandFollow(false);
                    pet->GetCharmInfo()->SetIsReturning(false);
                }
                pet->ToCreature()->AI()->AttackStart(blindeye);
            }
            return botAI->CastSpell("steady shot", olm);
        }
        break;

    case 1:
        if (!blindeye || !blindeyeTank)
            return false;

        botAI->CastSpell("misdirection", blindeyeTank);
        if (bot->HasAura(SPELL_MISDIRECTION))
            return botAI->CastSpell("steady shot", blindeye);
        break;

    default:
        break;
    }

    return false;
}

bool GruulsLairAutoPullTrashAction::Execute(Event event)
{
    if (!IsGruulsLairAutoPullReady(botAI, bot))
        return false;

    Unit* target = SelectGruulsLairTrashPullTarget(botAI, bot);
    if (!target)
        return false;

    context->GetValue<Unit*>("current target")->Set(target);
    bot->SetSelection(target->GetGUID());

    bool usedRangedOpener = false;
    if (botAI->IsTank(bot))
    {
        switch (bot->getClass())
        {
            case CLASS_PALADIN:
                usedRangedOpener = botAI->DoSpecificAction("avenger's shield", event, true) ||
                    botAI->DoSpecificAction("hand of reckoning", event, true);
                break;
            case CLASS_WARRIOR:
                usedRangedOpener = botAI->DoSpecificAction("heroic throw", event, true);
                break;
            case CLASS_DRUID:
                usedRangedOpener = botAI->DoSpecificAction("faerie fire (feral)", event, true) ||
                    botAI->DoSpecificAction("growl", event, true);
                break;
            case CLASS_DEATH_KNIGHT:
                usedRangedOpener = botAI->DoSpecificAction("death grip", event, true) ||
                    botAI->DoSpecificAction("icy touch", event, true) ||
                    botAI->DoSpecificAction("dark command", event, true);
                break;
            default:
                break;
        }
    }

    bool attackStarted = Attack(target);
    if (usedRangedOpener || attackStarted)
    {
        botAI->SetNextCheckDelay(sPlayerbotAIConfig.reactDelay);
        return true;
    }

    return false;
}

bool GruulsLairAutoPullTrashAction::isUseful()
{
    return IsGruulsLairAutoPullReady(botAI, bot) &&
           SelectGruulsLairTrashPullTarget(botAI, bot);
}

bool GruulsLairMarkBossPullReadyAction::Execute(Event /*event*/)
{
    MarkGruulsLairBossPullReady(botAI, bot);
    return true;
}

// Gruul the Dragonkiller Actions

// Position in center of the room
bool GruulTheDragonkillerTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* gruul = FindGruulTheDragonkiller(botAI, bot);
    if (!gruul)
        return false;

    if (gruul->GetMap())
    {
        const uint32 instanceId = gruul->GetMap()->GetInstanceId();
        if (!gruul->IsInCombat() || !gruulDpsWaitTimer.count(instanceId))
            gruulDpsWaitTimer[instanceId] = time(nullptr);
    }

    if (bot->GetVictim() != gruul)
        return Attack(gruul);

    if (gruul->GetVictim() == bot)
    {
        Position position = GRUUL_TANK_POSITION;
        if (botAI->IsAssistTankOfIndex(bot, 0, false))
            position.RelocateOffset({ 0.0f, -4.0f, 0.0f, 0.0f });

        const float maxDistance = 5.0f;

        float dX = position.GetPositionX() - bot->GetPositionX();
        float dY = position.GetPositionY() - bot->GetPositionY();
        float distanceToTankPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distanceToTankPosition > maxDistance)
        {
            float moveX = bot->GetPositionX() + (dX / distanceToTankPosition) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / distanceToTankPosition) * maxDistance;
            const float moveZ = position.GetPositionZ();
            return MoveTo(GRUULS_LAIR_MAP_ID, moveX, moveY, moveZ, false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// Ranged will take initial positions around the middle of the room, 25-40 yards from center
// Ranged should spread out 10 yards from each other
bool GruulTheDragonkillerSpreadRangedAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    static std::unordered_map<ObjectGuid, Position> initialPositions;
    static std::unordered_map<ObjectGuid, bool> hasReachedInitialPosition;

    Unit* gruul = FindGruulTheDragonkiller(botAI, bot);
    if (gruul && !gruul->IsInCombat() &&
        gruul->GetHealth() == gruul->GetMaxHealth())
    {
        initialPositions.erase(bot->GetGUID());
        hasReachedInitialPosition.erase(bot->GetGUID());
    }

    const Position& position = GRUUL_TANK_POSITION;
    const float centerX = position.GetPositionX();
    const float centerY = position.GetPositionY();
    const float centerZ = position.GetPositionZ();
    const float innerRadius = 27.0f;
    const float outerRadius = 34.0f;

    std::vector<Player*> rangedMembers;
    Player* closestMember = nullptr;
    float closestDist = std::numeric_limits<float>::max();
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if ((botAI->IsRanged(member) || botAI->IsHeal(member)) &&
            !botAI->IsTank(member))
            rangedMembers.push_back(member);

        if (member != bot)
        {
            float dist = bot->GetExactDist2d(member);
            if (dist < closestDist)
            {
                closestDist = dist;
                closestMember = member;
            }
        }
    }

    std::sort(rangedMembers.begin(), rangedMembers.end(),
              [](Player* first, Player* second)
              {
                  return first->GetGUID() < second->GetGUID();
              });

    if (rangedMembers.empty())
        return false;

    if (!initialPositions.count(bot->GetGUID()))
    {
        auto it = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
        size_t botIndex = (it != rangedMembers.end()) ?
            static_cast<size_t>(std::distance(rangedMembers.begin(), it)) : 0;
        size_t count = rangedMembers.size();

        if (count == 0)
            return false;

        float angle = (2.0f * static_cast<float>(M_PI) * static_cast<float>(botIndex)) /
                      static_cast<float>(count);
        float radius = (botIndex % 2 == 0) ? innerRadius : outerRadius;
        float targetX = centerX + radius * cos(angle);
        float targetY = centerY + radius * sin(angle);

        initialPositions[bot->GetGUID()] = Position(targetX, targetY, centerZ);
        hasReachedInitialPosition[bot->GetGUID()] = false;
    }

    Position targetPosition = initialPositions[bot->GetGUID()];
    if (!hasReachedInitialPosition[bot->GetGUID()])
    {
        if (!bot->IsWithinDist2d(targetPosition.GetPositionX(), targetPosition.GetPositionY(), 2.0f))
        {
            float destX = targetPosition.GetPositionX();
            float destY = targetPosition.GetPositionY();
            float destZ = targetPosition.GetPositionZ();

            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(),
                bot->GetPositionY(), bot->GetPositionZ(), destX, destY, destZ))
                return false;

            return MoveTo(GRUULS_LAIR_MAP_ID, destX, destY, destZ, false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        hasReachedInitialPosition[bot->GetGUID()] = true;
    }

    const float minSpreadDistance = 11.0f;
    const float movementThreshold = 2.0f;

    if (closestMember && closestDist < minSpreadDistance - movementThreshold)
    {
        return FleePosition(Position(closestMember->GetPositionX(), closestMember->GetPositionY(),
                            closestMember->GetPositionZ()), minSpreadDistance, 0);
    }

    return false;
}

// Try to get away from other group members when Ground Slam is cast
bool GruulTheDragonkillerShatterSpreadAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    GuidVector members = AI_VALUE(GuidVector, "group members");
    Unit* closestMember = nullptr;
    float closestDist = std::numeric_limits<float>::max();

    for (auto& member : members)
    {
        Unit* unit = botAI->GetUnit(member);
        if (!unit || bot->GetGUID() == member)
            continue;

        const float dist = bot->GetExactDist2d(unit);
        if (dist < closestDist)
        {
            closestDist = dist;
            closestMember = unit;
        }
    }

    if (closestMember && closestDist < 12.0f)
    {
        return FleePosition(Position(closestMember->GetPositionX(), closestMember->GetPositionY(),
                            closestMember->GetPositionZ()), 10.0f, 0);
    }

    return false;
}
