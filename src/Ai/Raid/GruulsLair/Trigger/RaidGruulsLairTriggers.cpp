#include "RaidGruulsLairTriggers.h"
#include "RaidGruulsLairHelpers.h"
#include "Playerbots.h"
#include <algorithm>

using namespace GruulsLairHelpers;

// High King Maulgar Triggers

bool HighKingMaulgarIsMainTankTrigger::IsActive()
{
    Unit* maulgar = FindHighKingMaulgar(botAI, bot);

    return botAI->IsMainTank(bot) && maulgar &&
           IsHighKingMaulgarPullAllowed(botAI, bot);
}

bool HighKingMaulgarIsFirstAssistTankTrigger::IsActive()
{
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");

    return botAI->IsAssistTankOfIndex(bot, 0, false) && olm &&
           IsHighKingMaulgarPullAllowed(botAI, bot);
}

bool HighKingMaulgarIsSecondAssistTankTrigger::IsActive()
{
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");

    return botAI->IsAssistTankOfIndex(bot, 1, false) && blindeye &&
           IsHighKingMaulgarPullAllowed(botAI, bot);
}

bool HighKingMaulgarIsMageTankTrigger::IsActive()
{
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");

    return IsKroshMageTank(botAI, bot) && krosh &&
           IsHighKingMaulgarPullAllowed(botAI, bot);
}

bool HighKingMaulgarIsMoonkinTankTrigger::IsActive()
{
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");

    return IsKigglerMoonkinTank(botAI, bot) && kiggler &&
           IsHighKingMaulgarPullAllowed(botAI, bot);
}

bool HighKingMaulgarDeterminingKillOrderTrigger::IsActive()
{
    Unit* maulgar = FindHighKingMaulgar(botAI, bot);
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");

    return IsAnyCouncilBossInCombat(botAI) &&
           botAI->IsDps(bot) &&
           !(botAI->IsMainTank(bot) && maulgar) &&
           !(botAI->IsAssistTankOfIndex(bot, 0, false) && olm) &&
           !(botAI->IsAssistTankOfIndex(bot, 1, false) && blindeye) &&
           !(IsKroshMageTank(botAI, bot) && krosh) &&
           !(IsKigglerMoonkinTank(botAI, bot) && kiggler);
}

bool HighKingMaulgarHealerInDangerTrigger::IsActive()
{
    return botAI->IsHeal(bot) && IsAnyCouncilBossInCombat(botAI) &&
           IsAnyOgreBossAlive(botAI);
}

bool HighKingMaulgarBlindeyeCastingTrigger::IsActive()
{
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    if (!blindeye || !blindeye->IsAlive() || !blindeye->IsInCombat() ||
        !blindeye->HasUnitState(UNIT_STATE_CASTING))
        return false;

    if (!blindeye->FindCurrentSpellBySpellId(SPELL_HEAL) &&
        !blindeye->FindCurrentSpellBySpellId(SPELL_PRAYER_OF_HEALING))
        return false;

    Value<std::list<uint32>>* spellIdsValue =
        botAI->GetAiObjectContext()->GetValue<std::list<uint32>>(
            "spell list", "interrupt");
    if (!spellIdsValue)
        return false;

    std::list<uint32> spellIds = spellIdsValue->Get();
    for (uint32 spellId : spellIds)
    {
        if (botAI->CanCastSpell(spellId, blindeye, false))
            return true;
    }

    return bot->IsWithinMeleeRange(blindeye);
}

bool HighKingMaulgarBossChannelingWhirlwindTrigger::IsActive()
{
    Unit* maulgar = FindHighKingMaulgar(botAI, bot);

    return maulgar && maulgar->HasAura(SPELL_WHIRLWIND) &&
           !botAI->IsMainTank(bot);
}

bool HighKingMaulgarWildFelstalkerSpawnedTrigger::IsActive()
{
    Unit* felStalker = AI_VALUE2(Unit*, "find target", "wild fel stalker");

    return felStalker && bot->getClass() == CLASS_WARLOCK &&
           IsHighKingMaulgarPullAllowed(botAI, bot);
}

bool HighKingMaulgarPullingOlmAndBlindeyeTrigger::IsActive()
{
    Group* group = bot->GetGroup();
    if (!group || bot->getClass() != CLASS_HUNTER)
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
    if (hunterIndex == -1 || hunterIndex > 1 ||
        !IsHighKingMaulgarPullAllowed(botAI, bot))
        return false;

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
        return olm && !olm->IsInCombat() &&
               olmTank && botAI->CanCastSpell("misdirection", olmTank);

    case 1:
        return blindeye && !blindeye->IsInCombat() &&
               (!olm || olm->IsInCombat()) &&
               blindeyeTank && botAI->CanCastSpell("misdirection", blindeyeTank);

    default:
        break;
    }

    return false;
}

bool GruulsLairAutoPullReadyTrigger::IsActive()
{
    return IsGruulsLairAutoPullReady(botAI, bot);
}

// Gruul the Dragonkiller Triggers

bool GruulTheDragonkillerBossEngagedByTanksTrigger::IsActive()
{
    Unit* gruul = FindGruulTheDragonkiller(botAI, bot);

    return gruul &&
           (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0, false)) &&
           IsGruulTheDragonkillerPullAllowed(botAI, bot);
}

bool GruulTheDragonkillerBossEngagedByRangedTrigger::IsActive()
{
    Unit* gruul = FindGruulTheDragonkiller(botAI, bot);

    return gruul && (botAI->IsRanged(bot) || botAI->IsHeal(bot)) &&
           IsGruulTheDragonkillerPullAllowed(botAI, bot);
}

bool GruulTheDragonkillerCaveInTrigger::IsActive()
{
    Unit* gruul = FindGruulTheDragonkiller(botAI, bot);
    Aura* areaDebuff = AI_VALUE(Aura*, "area debuff");

    return gruul && gruul->IsInCombat() && areaDebuff &&
           !botAI->IsTank(bot);
}

bool GruulTheDragonkillerIncomingShatterTrigger::IsActive()
{
    Unit* gruul = FindGruulTheDragonkiller(botAI, bot);

    return gruul && (bot->HasAura(SPELL_GROUND_SLAM_1) ||
           bot->HasAura(SPELL_GROUND_SLAM_2));
}
