#include "KarazhanMultipliers.h"
#include "KarazhanActions.h"
#include "KarazhanTriggers.h"
#include "ChooseTargetActions.h"
#include "GenericActions.h"
#include "AttackAction.h"
#include "Playerbots.h"
#include "PlayerbotMgr.h"
#include "Position.h"
#include <algorithm>
#include <cmath>
#include <ctime>
#include <tuple>

namespace
{
    static bool IsMovementAction(Action* action)
    {
        return dynamic_cast<MovementAction*>(action) != nullptr;
    }

    static std::vector<Player*> GetGroupMembers(Player* bot)
    {
        std::vector<Player*> members;
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member->IsAlive())
                    members.push_back(member);
            }
        }
        if (std::find(members.begin(), members.end(), bot) == members.end())
            members.push_back(bot);
        return members;
    }

    static std::vector<Player*> GetBlueBlockers(Player* bot, PlayerbotAI* botAI)
    {
        std::vector<Player*> blockers;
        for (Player* member : GetGroupMembers(bot))
        {
            if (!GET_PLAYERBOT_AI(member))
                continue;

            bool isDps = botAI->IsDps(member);
            bool isWarrior = member->getClass() == CLASS_WARRIOR;
            bool isRogue = member->getClass() == CLASS_ROGUE;
            bool hasExhaustion = member->HasAura(SPELL_EXHAUSTION_DOMINANCE);
            Aura* blueBuff = member->GetAura(SPELL_BLUE_BEAM_DEBUFF);
            bool overStack = blueBuff && blueBuff->GetStackAmount() >= 25;

            if (isDps && !isWarrior && !isRogue && !hasExhaustion && !overStack)
                blockers.push_back(member);
        }
        return blockers;
    }

    static std::vector<Player*> GetGreenBlockers(Player* bot, PlayerbotAI* botAI)
    {
        std::vector<Player*> blockers;
        for (Player* member : GetGroupMembers(bot))
        {
            if (!GET_PLAYERBOT_AI(member))
                continue;

            bool hasExhaustion = member->HasAura(SPELL_EXHAUSTION_SERENITY);
            Aura* greenBuff = member->GetAura(SPELL_GREEN_BEAM_DEBUFF);
            bool overStack = greenBuff && greenBuff->GetStackAmount() >= 25;
            bool isRogue = member->getClass() == CLASS_ROGUE;
            bool isDpsWarrior = member->getClass() == CLASS_WARRIOR && botAI->IsDps(member);
            bool eligibleRogueWarrior = (isRogue || isDpsWarrior) && !hasExhaustion;
            bool eligibleHealer = botAI->IsHeal(member) && !hasExhaustion && !overStack;

            if (eligibleRogueWarrior || eligibleHealer)
                blockers.push_back(member);
        }
        return blockers;
    }

    static Player* GetRedBlocker(Player* bot)
    {
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || !member->IsAlive())
                    continue;

                PlayerbotAI* memberAI = sPlayerbotsMgr->GetPlayerbotAI(member);
                if (!memberAI || !memberAI->IsTank(member))
                    continue;
                if (member->HasAura(SPELL_EXHAUSTION_PERSEVERANCE))
                    continue;

                return member;
            }
        }

        PlayerbotAI* botMemberAI = sPlayerbotsMgr->GetPlayerbotAI(bot);
        if (botMemberAI && botMemberAI->IsTank(bot) && !bot->HasAura(SPELL_EXHAUSTION_PERSEVERANCE))
            return bot;

        return nullptr;
    }

    static std::tuple<Player*, Player*, Player*> GetCurrentBeamBlockers(Player* bot, PlayerbotAI* botAI)
    {
        Player* red = GetRedBlocker(bot);
        std::vector<Player*> greenOptions = GetGreenBlockers(bot, botAI);
        std::vector<Player*> blueOptions = GetBlueBlockers(bot, botAI);
        Player* green = greenOptions.empty() ? nullptr : greenOptions.front();
        Player* blue = blueOptions.empty() ? nullptr : blueOptions.front();
        return std::make_tuple(red, green, blue);
    }

    static std::vector<Unit*> GetVoidZones(Player* bot, PlayerbotAI* botAI)
    {
        std::vector<Unit*> voidZones;
        GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
        for (ObjectGuid const& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->GetEntry() == NPC_VOID_ZONE)
                voidZones.push_back(unit);
        }
        return voidZones;
    }

    static Position GetBeamPosition(Unit* boss, Unit* portal, float distanceFromBoss)
    {
        if (!boss || !portal)
            return Position();

        float bx = boss->GetPositionX();
        float by = boss->GetPositionY();
        float bz = boss->GetPositionZ();
        float px = portal->GetPositionX();
        float py = portal->GetPositionY();

        float dx = px - bx;
        float dy = py - by;
        float length = std::sqrt(dx * dx + dy * dy);

        if (length == 0.0f)
            return Position(bx, by, bz);

        dx /= length;
        dy /= length;

        float targetX = bx + dx * distanceFromBoss;
        float targetY = by + dy * distanceFromBoss;
        return Position(targetX, targetY, bz);
    }

    static bool IsBotInsideBeam(Player* bot, Unit* boss, Unit* portal)
    {
        if (!boss || !portal)
            return false;

        float bx = boss->GetPositionX();
        float by = boss->GetPositionY();
        float px = portal->GetPositionX();
        float py = portal->GetPositionY();
        float dx = px - bx;
        float dy = py - by;
        float length = std::sqrt(dx * dx + dy * dy);
        if (length == 0.0f)
            return false;

        dx /= length;
        dy /= length;

        float botdx = bot->GetPositionX() - bx;
        float botdy = bot->GetPositionY() - by;
        float t = (botdx * dx + botdy * dy);
        if (t <= 0.0f || t >= length)
            return false;

        float beamX = bx + dx * t;
        float beamY = by + dy * t;
        float distToBeam = std::sqrt(std::pow(bot->GetPositionX() - beamX, 2.0f) +
                                     std::pow(bot->GetPositionY() - beamY, 2.0f));
        return distToBeam < 5.0f;
    }
}

float AttumenMultiplier::GetValue(Action* action)
{
    // Currently no specific multipliers needed
    // Can be added later if priority adjustments are needed
    return 1.0f;
}

float CuratorAddMultiplier::GetValue(Action* action)
{
    // Block DpsAssist when Astral Flares are present - prevents boss/add oscillation
    // Following proven pattern from UnderbogMultipliers.cpp:13-38
    if (botAI->IsHeal(bot)) { return 1.0f; }
    
    // Check for Astral Flares using same pattern as trigger
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    bool flarePresent = false;
    
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() && unit->GetEntry() == NPC_ASTRAL_FLARE)
        {
            flarePresent = true;
            break;
        }
    }
    
    if (flarePresent && dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f; // Block DpsAssist when flares present
    }
    
    return 1.0f;
}

float MoroesAddMultiplier::GetValue(Action* action)
{
    // Block DpsAssist when Moroes dinner guests are present - prevents boss/add oscillation
    if (botAI->IsHeal(bot)) { return 1.0f; }

    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    bool addPresent = false;

    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (!unit || !unit->IsInCombat())
            continue;

        uint32 entry = unit->GetEntry();
        if (entry == NPC_BARONESS_DOROTHEA || entry == NPC_LADY_CATRIONA || entry == NPC_LADY_KEIRA ||
            entry == NPC_LORD_ROBIN || entry == NPC_LORD_CRISPIN || entry == NPC_BARON_RAFE)
        {
            addPresent = true;
            break;
        }
    }

    if (addPresent && dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f; // Block DpsAssist when dinner guests are alive
    }

    return 1.0f;
}

float IllhoofAddMultiplier::GetValue(Action* action)
{
    // Block DpsAssist when Illhoof adds are present - prevents boss/add oscillation
    // Following proven pattern from UnderbogMultipliers.cpp:13-38
    if (botAI->IsHeal(bot)) { return 1.0f; }
    
    // Check for Illhoof adds using same pattern as trigger
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    bool addPresent = false;
    
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat())
        {
            uint32 entry = unit->GetEntry();
            // Priority: Kilrek > Demon Chains > Fiendish Imps
            if (entry == NPC_KILTREK || entry == NPC_DEMON_CHAINS || entry == NPC_FIENDISH_IMP)
            {
                addPresent = true;
                break;
            }
        }
    }
    
    if (addPresent && dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f; // Block DpsAssist when adds present
    }
    
    return 1.0f;
}

float MalchezaarAddMultiplier::GetValue(Action* action)
{
    // Block DpsAssist when Netherspite Infernals are present - prevents boss/add oscillation
    // Following proven pattern from CLAUDE.md:678-702
    if (botAI->IsHeal(bot)) { return 1.0f; }
    
    // Check for spawned infernals using same pattern as trigger
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    bool infernalPresent = false;
    
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() && unit->GetEntry() == NPC_NETHERSPITE_INFERNAL)
        {
            infernalPresent = true;
            break;
        }
    }
    
    if (infernalPresent && dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f; // Block DpsAssist when infernals present
    }
    
    return 1.0f;
}

float NightbaneAddMultiplier::GetValue(Action* action)
{
    // Block DpsAssist when Restless Skeletons are present - prevents boss/add oscillation
    // Following proven pattern from CLAUDE.md:678-702
    if (botAI->IsHeal(bot)) { return 1.0f; }
    
    // Check for spawned skeletons using same pattern as trigger
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    bool skeletonPresent = false;
    
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() && unit->GetEntry() == NPC_RESTLESS_SKELETON)
        {
            skeletonPresent = true;
            break;
        }
    }
    
    if (skeletonPresent && dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f; // Block DpsAssist when skeletons present
    }

    return 1.0f;
}

float NetherspiteBlueAndGreenBeamMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!boss || !boss->IsAlive())
        return 1.0f;

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(bot, botAI);
    bool isBeamBlocker = (bot == greenBlocker || bot == blueBlocker);
    if (!isBeamBlocker)
        return 1.0f;

    Unit* bluePortal = bot->FindNearestCreature(NPC_NETHER_PORTAL_DOMINANCE, 150.0f);
    Unit* greenPortal = bot->FindNearestCreature(NPC_NETHER_PORTAL_SERENITY, 150.0f);

    bool inBeam = IsBotInsideBeam(bot, boss, bluePortal) || IsBotInsideBeam(bot, boss, greenPortal);
    if (!inBeam)
        return 1.0f;

    std::vector<Unit*> voidZones = GetVoidZones(bot, botAI);
    bool inVoidZone = std::any_of(voidZones.begin(), voidZones.end(), [&](Unit* vz)
    {
        return vz && bot->GetExactDist2d(vz) < 4.0f;
    });

    if (inVoidZone)
        return 1.0f;

    if (IsMovementAction(action))
        return 0.0f;

    return 1.0f;
}

float NetherspiteRedBeamMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!boss || !boss->IsAlive())
        return 1.0f;

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(bot, botAI);
    if (bot != redBlocker)
        return 1.0f;

    Unit* redPortal = bot->FindNearestCreature(NPC_NETHER_PORTAL_PERSEVERANCE, 150.0f);
    if (!redPortal)
        return 1.0f;

    Position beamPos = GetBeamPosition(boss, redPortal, 18.0f);
    float bx = boss->GetPositionX();
    float by = boss->GetPositionY();
    float px = redPortal->GetPositionX();
    float py = redPortal->GetPositionY();
    float dx = px - bx;
    float dy = py - by;
    float length = std::sqrt(dx * dx + dy * dy);
    if (length == 0.0f)
        return 1.0f;

    dx /= length;
    dy /= length;
    float perpDx = -dy;
    float perpDy = dx;
    Position sidewaysPos(beamPos.GetPositionX() + perpDx * 3.0f,
                         beamPos.GetPositionY() + perpDy * 3.0f,
                         beamPos.GetPositionZ());

    static std::map<ObjectGuid, uint32> beamMoveTimes;
    static std::map<ObjectGuid, bool> lastBeamMoveSideways;
    ObjectGuid botGuid = bot->GetGUID();
    uint32 intervalSecs = 5;

    if (beamMoveTimes[botGuid] == 0)
    {
        beamMoveTimes[botGuid] = time(nullptr);
        lastBeamMoveSideways[botGuid] = false;
    }

    if (time(nullptr) - beamMoveTimes[botGuid] >= intervalSecs)
    {
        lastBeamMoveSideways[botGuid] = !lastBeamMoveSideways[botGuid];
        beamMoveTimes[botGuid] = time(nullptr);
    }

    Position targetPos = lastBeamMoveSideways[botGuid] ? sidewaysPos : beamPos;
    float distToTarget = bot->GetExactDist2d(targetPos.GetPositionX(), targetPos.GetPositionY());
    const float positionTolerance = 1.5f;

    if (distToTarget < positionTolerance && IsMovementAction(action))
        return 0.0f;

    return 1.0f;
}
