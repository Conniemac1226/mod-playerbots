#include "SerpentshrineActions.h"
#include "SerpentshrineTriggers.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "ScriptedCreature.h"
#include "GameObject.h"
#include "Group.h"
#include "Map.h"
#include "MotionMaster.h"
#include "Unit.h"
#include "SpellInfo.h"
#include "SharedDefines.h"
#include "Spell.h"
#include "UseItemAction.h"
#include "AiObjectContext.h"
#include "Value.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <vector>

namespace
{
    constexpr float LURKER_PLATFORM_Z = -19.18f;

    inline float GetPersonalOffset(Player* bot, float step)
    {
        if (!bot)
            return 0.0f;

        int32 bucket = static_cast<int32>(bot->GetGUID().GetCounter() % 8);
        bucket -= 4;
        return static_cast<float>(bucket) * step;
    }

    bool IsCastingSpell(Unit* caster, uint32 spellId)
    {
        if (!caster)
            return false;

        for (uint32 i = CURRENT_MELEE_SPELL; i <= CURRENT_CHANNELED_SPELL; ++i)
        {
            CurrentSpellTypes type = static_cast<CurrentSpellTypes>(i);
            if (Spell* spell = caster->GetCurrentSpell(type))
            {
                if (spell->m_spellInfo && spell->m_spellInfo->Id == spellId)
                    return true;
            }
        }

        return false;
    }

    // Manual spread contributions per mechanic so we can honour the shared disperse-distance system.
    constexpr std::array<char const*, 7> s_serpentshrineSpreadKeys = {
        "hydross water tomb spread distance",
        "hydross vile sludge spread distance",
        "lurker geyser spread distance",
        "leotheras chaos blast spread distance",
        "karathress sear nova spread distance",
        "karathress cyclone spread distance",
        "vashj static charge spread distance"
    };

    constexpr std::array<uint32, 4> s_vashjShieldGenerators = {
        185051,
        185052,
        185053,
        185054
    };

    const std::array<Position, 4> s_vashjShieldGeneratorPositions = {
        Position(7.81f, -945.244f, 44.0f, 5.99871f),
        Position(52.048f, -901.236f, 44.0f, 3.02393f),
        Position(7.417f, -901.109f, 44.0f, 2.29077f),
        Position(52.448f, -944.825f, 44.0f, 3.48714f)
    };

    constexpr char const* HYDROSS_LAST_MOVE_TIME = "hydross last move time";
    constexpr char const* HYDROSS_MARK_STACKS = "hydross mark stacks";
    constexpr char const* HYDROSS_TRANSITION_NEEDED = "hydross transition needed";
    constexpr char const* HYDROSS_ACTIVE_ADD = "hydross active add";
    constexpr std::array<uint32, 2> s_hydrossAddEntries = {
        NPC_PURE_SPAWN_OF_HYDROSS,
        NPC_TAINTED_SPAWN_OF_HYDROSS
    };
    constexpr uint8 HYDROSS_SKULL_ICON_INDEX = 7;
    constexpr char const* LEOTHERAS_SHADOW_TARGET = "leotheras shadow target";
    constexpr char const* KARATHRESS_CYCLONE_TARGET = "karathress cyclone target";
    constexpr char const* KARATHRESS_TOTEM_TARGET = "karathress totem target";
    constexpr char const* KARATHRESS_CYCLONE_SPREAD_DISTANCE = "karathress cyclone spread distance";

    constexpr char const* LURKER_LAST_SPOUT_TIME = "lurker last spout time";
    constexpr char const* LURKER_IN_WATER = "lurker in water";
    constexpr char const* LURKER_SPOUT_BASE_ANGLE = "lurker spout base angle";

    constexpr char const* LEOTHERAS_LAST_WHIRLWIND_TIME = "leotheras last whirlwind time";
    constexpr char const* LEOTHERAS_HAS_DEMON = "leotheras has demon";
    constexpr char const* LEOTHERAS_WHIRLWIND_HOLD_UNTIL = "leotheras whirlwind hold until";

    constexpr char const* MOROGRIM_LAST_GRAVE_TIME = "morogrim last grave time";

    class TaintedCoreUseAction : public UseItemAction
    {
    public:
        explicit TaintedCoreUseAction(PlayerbotAI* botAI) : UseItemAction(botAI, "tainted core", true) {}
        using UseItemAction::UseItemOnGameObject;
    };

    void UpdateSerpentshrineDisperseDistance(PlayerbotAI* botAI)
    {
        if (!botAI)
            return;

        float desired = 0.0f;
        for (char const* key : s_serpentshrineSpreadKeys)
        {
            if (Value<float>* contribution = botAI->GetAiObjectContext()->GetValue<float>(key))
            {
                desired = std::max(desired, contribution->Get());
            }
        }

        Value<float>* disperseValue = botAI->GetAiObjectContext()->GetValue<float>("disperse distance");
        if (!disperseValue)
            return;

        if (std::fabs(disperseValue->Get() - desired) > 0.1f)
        {
            disperseValue->Set(desired);
        }
    }

    Unit* SelectHydrossAdd(PlayerbotAI* botAI, Player* bot, GuidVector const& candidates, float& currentBestDistance)
    {
        if (!botAI || !bot)
            return nullptr;

        Unit* best = nullptr;

        for (ObjectGuid const& guid : candidates)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsAlive())
                continue;

            bool isHydrossAdd = std::any_of(s_hydrossAddEntries.begin(), s_hydrossAddEntries.end(),
                                             [unit](uint32 entry) { return unit->GetEntry() == entry; });
            if (!isHydrossAdd)
                continue;

            float distance = bot->GetDistance(unit);
            if (distance < currentBestDistance)
            {
                currentBestDistance = distance;
                best = unit;
            }
        }

        return best;
    }

    void UpdateHydrossAddMarker(PlayerbotAI* botAI, Player* bot, Unit* target)
    {
        if (!botAI || !bot || !target)
            return;

        if (Group* group = bot->GetGroup())
        {
            ObjectGuid currentIcon = group->GetTargetIcon(HYDROSS_SKULL_ICON_INDEX);
            Unit* currentUnit = botAI->GetUnit(currentIcon);
            if (currentUnit && currentUnit->IsAlive() && currentUnit == target)
                return;

            group->SetTargetIcon(HYDROSS_SKULL_ICON_INDEX, bot->GetGUID(), target->GetGUID());
        }
    }

    Unit* SelectLeotherasShadow(PlayerbotAI* botAI, Player* bot, GuidVector const& candidates, float& currentBestDistance)
    {
        if (!botAI || !bot)
            return nullptr;

        Unit* best = nullptr;

        for (ObjectGuid const& guid : candidates)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsAlive() || unit->GetEntry() != NPC_SHADOW_OF_LEOTHERAS)
                continue;

            float distance = bot->GetDistance(unit);
            if (distance < currentBestDistance)
            {
                currentBestDistance = distance;
                best = unit;
            }
        }

        return best;
    }

    Unit* ResolveLeotherasShadow(PlayerbotAI* botAI, Player* bot, Value<ObjectGuid>* focusValue)
    {
        if (!botAI || !bot)
            return nullptr;

        float bestDistance = std::numeric_limits<float>::max();
        Unit* shadow = nullptr;

        if (focusValue && !focusValue->Get().IsEmpty())
        {
            if (Unit* focusShadow = botAI->GetUnit(focusValue->Get()))
            {
                if (focusShadow->IsAlive())
                {
                    shadow = focusShadow;
                    bestDistance = bot->GetDistance(focusShadow);
                }
                else
                {
                    focusValue->Set(ObjectGuid::Empty);
                }
            }
            else
            {
                focusValue->Set(ObjectGuid::Empty);
            }
        }

        auto consider = [&](GuidVector const& vec)
        {
            if (vec.empty())
                return;

            if (Unit* candidate = SelectLeotherasShadow(botAI, bot, vec, bestDistance))
            {
                shadow = candidate;
            }
        };

        if (Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs"))
        {
            consider(npcsValue->Get());
        }

        if (!shadow)
        {
            if (Value<GuidVector>* noLosValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los"))
            {
                consider(noLosValue->Get());
            }
        }

        if (!shadow)
        {
            if (Value<GuidVector>* possibleValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets"))
            {
                consider(possibleValue->Get());
            }
        }

        if (shadow && focusValue)
        {
            focusValue->Set(shadow->GetGUID());
        }

        return shadow;
    }

    Unit* SelectKarathressCyclone(PlayerbotAI* botAI, Player* bot, GuidVector const& candidates, float& currentBestDistance)
    {
        if (!botAI || !bot)
            return nullptr;

        Unit* best = nullptr;

        for (ObjectGuid const& guid : candidates)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsAlive() || unit->GetEntry() != NPC_CYCLONE_KARATHRESS)
                continue;

            float distance = bot->GetDistance(unit);
            if (distance < currentBestDistance)
            {
                currentBestDistance = distance;
                best = unit;
            }
        }

        return best;
    }

    void SetSerpentshrinePriority(PlayerbotAI* botAI, Unit* unit)
    {
        if (!botAI || !unit)
            return;

        if (Value<GuidVector>* prioritized = botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets"))
        {
            GuidVector current = prioritized->Get();
            if (current.size() == 1 && current.front() == unit->GetGUID())
                return;

            GuidVector updated;
            updated.push_back(unit->GetGUID());
            prioritized->Set(updated);
        }
    }

    void ClearSerpentshrinePriority(PlayerbotAI* botAI, ObjectGuid const& guid = ObjectGuid::Empty)
    {
        if (!botAI)
            return;

        if (Value<GuidVector>* prioritized = botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets"))
        {
            GuidVector current = prioritized->Get();
            if (current.empty())
                return;

            if (!guid.IsEmpty())
            {
                if (current.size() != 1 || current.front() != guid)
                    return;
            }

            prioritized->Set(GuidVector());
        }
    }
}

bool HydrossAvoidMarkOfHydrossAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    uint8 stacks = 0;
    if (bot->HasAura(SPELL_MARK_OF_HYDROSS1)) stacks = 1;
    else if (bot->HasAura(SPELL_MARK_OF_HYDROSS2)) stacks = 2;
    else if (bot->HasAura(SPELL_MARK_OF_HYDROSS3)) stacks = 3;
    else if (bot->HasAura(SPELL_MARK_OF_HYDROSS4)) stacks = 4;
    else if (bot->HasAura(SPELL_MARK_OF_HYDROSS5)) stacks = 5;
    else if (bot->HasAura(SPELL_MARK_OF_HYDROSS6)) stacks = 6;

    Value<uint8>* markValue = botAI->GetAiObjectContext()->GetValue<uint8>(HYDROSS_MARK_STACKS);
    if (markValue)
    {
        markValue->Set(stacks);
    }

    if (botAI->IsTank(bot) && stacks >= 4)
    {
        if (Value<bool>* transitionNeeded = botAI->GetAiObjectContext()->GetValue<bool>(HYDROSS_TRANSITION_NEEDED))
        {
            transitionNeeded->Set(true);
        }
        return false;
    }

    if (!botAI->IsTank(bot) && stacks >= 3)
    {
        float distance = bot->GetDistance(boss);
        if (distance < 20.0f)
        {
            Position movePos = bot->GetPosition();
            float angle = bot->GetAngle(boss) + M_PI;
            movePos.m_positionX += cos(angle) * 10.0f;
            movePos.m_positionY += sin(angle) * 10.0f;
            
            return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                         false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
        }
    }

    return false;
}

bool HydrossAvoidMarkOfCorruptionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    uint8 stacks = 0;
    if (bot->HasAura(SPELL_MARK_OF_CORRUPTION1)) stacks = 1;
    else if (bot->HasAura(SPELL_MARK_OF_CORRUPTION2)) stacks = 2;
    else if (bot->HasAura(SPELL_MARK_OF_CORRUPTION3)) stacks = 3;
    else if (bot->HasAura(SPELL_MARK_OF_CORRUPTION4)) stacks = 4;
    else if (bot->HasAura(SPELL_MARK_OF_CORRUPTION5)) stacks = 5;
    else if (bot->HasAura(SPELL_MARK_OF_CORRUPTION6)) stacks = 6;

    if (Value<uint8>* markValue = botAI->GetAiObjectContext()->GetValue<uint8>(HYDROSS_MARK_STACKS))
    {
        markValue->Set(stacks);
    }

    if (botAI->IsTank(bot) && stacks >= 4)
    {
        if (Value<bool>* transitionNeeded = botAI->GetAiObjectContext()->GetValue<bool>(HYDROSS_TRANSITION_NEEDED))
        {
            transitionNeeded->Set(true);
        }
        return false;
    }

    if (!botAI->IsTank(bot) && stacks >= 3)
    {
        float distance = bot->GetDistance(boss);
        if (distance < 20.0f)
        {
            Position movePos = bot->GetPosition();
            float angle = bot->GetAngle(boss) + M_PI;
            movePos.m_positionX += cos(angle) * 10.0f;
            movePos.m_positionY += sin(angle) * 10.0f;
            
            return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                         false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
        }
    }

    return false;
}

bool HydrossWaterTombSpreadAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Value<float>* spreadValue = botAI->GetAiObjectContext()->GetValue<float>("hydross water tomb spread distance");
    if (!spreadValue)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        if (spreadValue->Get() > 0.0f)
        {
            spreadValue->Set(0.0f);
            UpdateSerpentshrineDisperseDistance(botAI);
        }
        return false;
    }

    bool shouldSpread = false;

    // React immediately to cast or pre-cast
    if (boss->FindCurrentSpellBySpellId(SPELL_WATER_TOMB) ||
        (!boss->HasAura(SPELL_HYDROSS_CORRUPTION) && boss->HasUnitState(UNIT_STATE_CASTING)))
    {
        Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
        if (membersValue)
        {
            GuidVector members = membersValue->Get();
            for (ObjectGuid const& member : members)
            {
                Unit* player = botAI->GetUnit(member);
                if (player && player != bot && player->IsAlive() && bot->GetDistance(player) < 8.0f)
                {
                    shouldSpread = true;
                    break;
                }
            }
        }
    }

    float desiredDistance = botAI->IsMelee(bot) ? 9.0f : 18.0f;
    if (shouldSpread)
    {
        if (spreadValue->Get() < desiredDistance)
        {
            spreadValue->Set(desiredDistance);
            UpdateSerpentshrineDisperseDistance(botAI);
        }

        Position movePos = boss->GetPosition();
        float angle = Position::NormalizeOrientation(boss->GetOrientation() + M_PI + GetPersonalOffset(bot, 0.1f));
        movePos.m_positionX += cos(angle) * desiredDistance;
        movePos.m_positionY += sin(angle) * desiredDistance;
        float targetZ = movePos.m_positionZ;
        bot->UpdateAllowedPositionZ(movePos.m_positionX, movePos.m_positionY, targetZ);
        movePos.m_positionZ = targetZ;

        float distanceToMove = bot->GetDistance(movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ);
        if (distanceToMove < 1.5f)
        {
            return false;
        }

        if (MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                   false, false, false, false, MovementPriority::MOVEMENT_NORMAL))
        {
            botAI->SetNextCheckDelay(250);
            return true;
        }

        return false;
    }

    if (spreadValue->Get() > 0.0f)
    {
        spreadValue->Set(0.0f);
        UpdateSerpentshrineDisperseDistance(botAI);
    }

    return false;
}

bool HydrossVileSludgeSpreadAction::Execute(Event event)
{
    Value<float>* spreadValue = botAI->GetAiObjectContext()->GetValue<float>("hydross vile sludge spread distance");
    if (!spreadValue)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    bool bossActive = boss && boss->IsAlive() && boss->IsInCombat();
    bool hasSludgeAura = bot->HasAura(SPELL_VILE_SLUDGE);

    if (!bossActive && !hasSludgeAura)
    {
        if (spreadValue->Get() > 0.0f)
        {
            spreadValue->Set(0.0f);
            UpdateSerpentshrineDisperseDistance(botAI);
        }
        return false;
    }

    bool shouldSpread = hasSludgeAura;

    // React to cast or pre-cast in poison phase
    if (bossActive && boss->HasAura(SPELL_HYDROSS_CORRUPTION))
    {
        float distance = bot->GetDistance(boss);
        
        // Immediate reaction to Vile Sludge cast
        if (boss->FindCurrentSpellBySpellId(SPELL_VILE_SLUDGE))
        {
            shouldSpread = shouldSpread || distance < 20.0f;
        }
        // Pre-emptive movement when boss is casting in poison form
        else if (boss->HasUnitState(UNIT_STATE_CASTING))
        {
            shouldSpread = shouldSpread || distance < 12.0f;
        }
    }

    float desiredDistance = botAI->IsMelee(bot) ? 9.0f : 18.0f;
    if (shouldSpread)
    {
        if (spreadValue->Get() < desiredDistance)
        {
            spreadValue->Set(desiredDistance);
            UpdateSerpentshrineDisperseDistance(botAI);
        }

        Position movePos = boss->GetPosition();
        float angle = Position::NormalizeOrientation(boss->GetOrientation() + M_PI + GetPersonalOffset(bot, 0.1f));
        movePos.m_positionX += cos(angle) * desiredDistance;
        movePos.m_positionY += sin(angle) * desiredDistance;
        float targetZ = movePos.m_positionZ;
        bot->UpdateAllowedPositionZ(movePos.m_positionX, movePos.m_positionY, targetZ);
        movePos.m_positionZ = targetZ;

        float distanceToMove = bot->GetDistance(movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ);
        if (distanceToMove < 1.5f)
        {
            return false;
        }

        if (MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                   false, false, false, false, MovementPriority::MOVEMENT_NORMAL))
        {
            botAI->SetNextCheckDelay(250);
            return true;
        }

        return false;
    }

    if (spreadValue->Get() > 0.0f)
    {
        spreadValue->Set(0.0f);
        UpdateSerpentshrineDisperseDistance(botAI);
    }

    return false;
}

bool HydrossKillAddsAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Value<ObjectGuid>* activeAddValue = botAI->GetAiObjectContext()->GetValue<ObjectGuid>(HYDROSS_ACTIVE_ADD);
    ObjectGuid activeGuid = ObjectGuid::Empty;
    Unit* activeTarget = nullptr;
    if (activeAddValue)
    {
        activeGuid = activeAddValue->Get();
        if (!activeGuid.IsEmpty())
        {
            if (Unit* unit = botAI->GetUnit(activeGuid))
            {
                if (unit->IsAlive())
                {
                    activeTarget = unit;
                }
                else
                {
                    activeAddValue->Set(ObjectGuid::Empty);
                }
            }
            else
            {
                activeAddValue->Set(ObjectGuid::Empty);
            }
        }
    }

    Unit* target = activeTarget;
    float bestDistance = target ? bot->GetDistance(target) : 200.0f;

    if (!target)
    {
        if (Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs"))
        {
            target = SelectHydrossAdd(botAI, bot, npcsValue->Get(), bestDistance);
        }
    }

    if (!target)
    {
        if (Value<GuidVector>* noLosValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los"))
        {
            target = SelectHydrossAdd(botAI, bot, noLosValue->Get(), bestDistance);
        }
    }

    if (!target)
    {
        if (Value<GuidVector>* possibleValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets"))
        {
            target = SelectHydrossAdd(botAI, bot, possibleValue->Get(), bestDistance);
        }
    }

    if (target)
    {
        float distanceToTarget = bot->GetDistance(target);
        bool hasLineOfSight = bot->IsWithinLOSInMap(target);

        if (!hasLineOfSight || (botAI->IsMelee(bot) && distanceToTarget > 5.0f))
        {
            Position movePos = target->GetPosition();
            if (!botAI->IsMelee(bot))
            {
                float angle = target->GetAngle(bot);
                float desiredRange = 18.0f;
                movePos.m_positionX += std::cos(angle) * desiredRange;
                movePos.m_positionY += std::sin(angle) * desiredRange;
            }

            float adjustedZ = movePos.m_positionZ;
            bot->UpdateAllowedPositionZ(movePos.m_positionX, movePos.m_positionY, adjustedZ);
            movePos.m_positionZ = adjustedZ;

            if (MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                       false, false, false, false, MovementPriority::MOVEMENT_NORMAL))
            {
                botAI->SetNextCheckDelay(200);
                return true;
            }
        }

        if (AI_VALUE(Unit*, "current target") != target)
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(target);
        }

        if (activeAddValue && activeAddValue->Get() != target->GetGUID())
        {
            activeAddValue->Set(target->GetGUID());
        }

        UpdateHydrossAddMarker(botAI, bot, target);

        if (Attack(target))
        {
            botAI->SetNextCheckDelay(150);
            return true;
        }

        return false;
    }

    if (activeAddValue && !activeGuid.IsEmpty())
    {
        activeAddValue->Set(ObjectGuid::Empty);
    }

    return false;
}

bool HydrossPositionTankAction::Execute(Event event)
{
    if (!botAI->IsTank(bot))
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    bool hasCorruption = boss->HasAura(SPELL_HYDROSS_CORRUPTION);
    bool hasBlueBeam = boss->HasAura(SPELL_BLUE_BEAM);
    
    float desiredX, desiredY, desiredZ;
    
    if (hasCorruption)
    {
        desiredX = -29.0f;
        desiredY = -923.0f;
        desiredZ = 42.0f;
    }
    else
    {
        desiredX = 71.0f;
        desiredY = -883.0f;
        desiredZ = 41.0f;
    }
    
    Position desiredPos(desiredX, desiredY, desiredZ);
    float distance = bot->GetDistance(desiredPos);
    
    if (distance > 5.0f)
    {
        return MoveTo(bot->GetMapId(), desiredX, desiredY, desiredZ,
                     false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
    }
    
    return false;
}

bool HydrossTransitionControlAction::Execute(Event event)
{
    if (!botAI->IsTank(bot))
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    Value<bool>* transitionNeededValue = botAI->GetAiObjectContext()->GetValue<bool>(HYDROSS_TRANSITION_NEEDED);
    if (!transitionNeededValue || !transitionNeededValue->Get())
    {
        return false;
    }

    bool hasCorruption = boss->HasAura(SPELL_HYDROSS_CORRUPTION);
    
    float targetX, targetY, targetZ;
    
    if (hasCorruption)
    {
        targetX = 71.0f;
        targetY = -883.0f;
        targetZ = 41.0f;
    }
    else
    {
        targetX = -29.0f;
        targetY = -923.0f;
        targetZ = 42.0f;
    }
    
    Value<uint32>* lastMoveValue = botAI->GetAiObjectContext()->GetValue<uint32>(HYDROSS_LAST_MOVE_TIME);
    Value<uint8>* markStacksValue = botAI->GetAiObjectContext()->GetValue<uint8>(HYDROSS_MARK_STACKS);

    uint32 currentTime = getMSTime();
    uint32 lastMoveTime = lastMoveValue ? lastMoveValue->Get() : 0;
    if (lastMoveValue && lastMoveTime != 0 && currentTime - lastMoveTime > 10000)
    {
        transitionNeededValue->Set(false);
        if (markStacksValue)
        {
            markStacksValue->Set(0);
        }
        lastMoveValue->Set(currentTime);
        return false;
    }

    if (lastMoveValue)
    {
        lastMoveValue->Set(currentTime);
    }

    return MoveTo(bot->GetMapId(), targetX, targetY, targetZ,
                 false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
}

bool LurkerSpoutAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    Value<uint32>* lastSpoutValue = botAI->GetAiObjectContext()->GetValue<uint32>(LURKER_LAST_SPOUT_TIME);
    Value<bool>* inWaterValue = botAI->GetAiObjectContext()->GetValue<bool>(LURKER_IN_WATER);
    Value<float>* baseAngleValue = botAI->GetAiObjectContext()->GetValue<float>(LURKER_SPOUT_BASE_ANGLE);

    // Check if boss is casting or has spout visual
    if (boss->HasAura(SPELL_LURKER_SPOUT_VISUAL) ||
        boss->FindCurrentSpellBySpellId(SPELL_LURKER_SPOUT_PERIODIC_1) ||
        boss->FindCurrentSpellBySpellId(SPELL_LURKER_SPOUT_PERIODIC_2))
    {
        uint32 currentTime = getMSTime();

        float platformZ = LURKER_PLATFORM_Z;
        float radius = botAI->IsMelee(bot) ? 9.0f : 24.0f;

        bool moveClockwise = (bot->GetGUID().GetCounter() % 2) == 0;
        float direction = moveClockwise ? 1.0f : -1.0f;

        float baseAngle = baseAngleValue ? baseAngleValue->Get() : 0.0f;
        uint32 spoutStart = lastSpoutValue ? lastSpoutValue->Get() : 0;
        bool newSpout = spoutStart == 0 || currentTime < spoutStart;
        if (newSpout)
        {
            spoutStart = currentTime;
            float behindAngle = Position::NormalizeOrientation(boss->GetOrientation() + M_PI);
            baseAngle = Position::NormalizeOrientation(behindAngle + GetPersonalOffset(bot, 0.12f));
            if (baseAngleValue)
                baseAngleValue->Set(baseAngle);
            if (lastSpoutValue)
                lastSpoutValue->Set(spoutStart);
        }
        else if (baseAngle == 0.0f && baseAngleValue)
        {
            float behindAngle = Position::NormalizeOrientation(boss->GetOrientation() + M_PI);
            baseAngle = Position::NormalizeOrientation(behindAngle + GetPersonalOffset(bot, 0.12f));
            baseAngleValue->Set(baseAngle);
        }

        float elapsed = (spoutStart > 0) ? static_cast<float>(currentTime - spoutStart) / 1000.0f : 0.0f;
        float angularSpeed = botAI->IsMelee(bot) ? 3.6f : 4.8f; // radians per second
        float targetAngle = baseAngle + direction * angularSpeed * elapsed;

        Position targetPos = boss->GetPosition();
        targetPos.m_positionX += cos(targetAngle) * radius;
        targetPos.m_positionY += sin(targetAngle) * radius;
        targetPos.m_positionZ = platformZ;

        float adjustedZ = targetPos.m_positionZ;
        bot->UpdateAllowedPositionZ(targetPos.m_positionX, targetPos.m_positionY, adjustedZ);
        targetPos.m_positionZ = adjustedZ;

        if (inWaterValue)
        {
            inWaterValue->Set(true);
        }

        bot->GetMotionMaster()->Clear(false);
        return MoveTo(bot->GetMapId(), targetPos.m_positionX, targetPos.m_positionY, targetPos.m_positionZ,
                     false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }
    else if (inWaterValue && inWaterValue->Get())
    {
        // Spout ended, get back on platform
        inWaterValue->Set(false);
        if (lastSpoutValue)
            lastSpoutValue->Set(0);
        if (baseAngleValue)
            baseAngleValue->Set(0.0f);

        float platformZ = LURKER_PLATFORM_Z;
        float radius = botAI->IsMelee(bot) ? 9.0f : 24.0f;
        Position targetPos = boss ? boss->GetPosition() : bot->GetPosition();
        float angle = boss ? Position::NormalizeOrientation(boss->GetOrientation() + M_PI) : bot->GetOrientation();
        angle += GetPersonalOffset(bot, 0.12f);
        targetPos.m_positionX += cos(angle) * radius;
        targetPos.m_positionY += sin(angle) * radius;
        float adjustedZ = platformZ;
        bot->UpdateAllowedPositionZ(targetPos.m_positionX, targetPos.m_positionY, adjustedZ);
        targetPos.m_positionZ = adjustedZ;

        return MoveTo(bot->GetMapId(), targetPos.m_positionX, targetPos.m_positionY, targetPos.m_positionZ,
                     false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }

    return false;
}

bool LurkerWhirlAvoidAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Only melee needs to avoid whirl
    if (!botAI->IsMelee(bot))
    {
        return false;
    }

    // Check if boss is casting whirl
    if (boss->FindCurrentSpellBySpellId(SPELL_LURKER_WHIRL) || boss->HasAura(SPELL_LURKER_WHIRL))
    {
        float distance = bot->GetDistance(boss);
        if (distance < 8.0f) // Whirl has 5 yard range + buffer
        {
            Position movePos = bot->GetPosition();
            float angle = bot->GetAngle(boss) + M_PI;
            movePos.m_positionX += cos(angle) * 10.0f;
            movePos.m_positionY += sin(angle) * 10.0f;
            
            return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                         false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
        }
    }

    return false;
}

bool LurkerGeyserSpreadAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Value<float>* spreadValue = botAI->GetAiObjectContext()->GetValue<float>("lurker geyser spread distance");
    if (!spreadValue)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        if (spreadValue->Get() > 0.0f)
        {
            spreadValue->Set(0.0f);
            UpdateSerpentshrineDisperseDistance(botAI);
        }
        return false;
    }

    // Check if boss is casting geyser
    if (boss->FindCurrentSpellBySpellId(SPELL_LURKER_GEYSER))
    {
        constexpr float desiredDistance = 12.0f;
        if (spreadValue->Get() < desiredDistance)
        {
            spreadValue->Set(desiredDistance);
            UpdateSerpentshrineDisperseDistance(botAI);
        }
        botAI->SetNextCheckDelay(250);
        return true;
    }

    if (spreadValue->Get() > 0.0f)
    {
        spreadValue->Set(0.0f);
        UpdateSerpentshrineDisperseDistance(botAI);
    }

    return false;
}

bool LurkerKillAddsAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Only attack adds when boss is submerged
    if (!boss->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
    {
        return false;
    }


    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
    {
        return false;
    }
    GuidVector npcs = npcsValue->Get();
    
    Unit* guardian = nullptr;
    Unit* ambusher = nullptr;
    float minGuardianDistance = 100.0f;
    float minAmbusherDistance = 100.0f;

    for (ObjectGuid const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_COILFANG_FRENZY)
            continue;

        float distance = bot->GetDistance(unit);
        
        // Guardians are priority (harder hitting)
        if (unit->GetEntry() == NPC_COILFANG_GUARDIAN && distance < minGuardianDistance)
        {
            guardian = unit;
            minGuardianDistance = distance;
        }
        else if (unit->GetEntry() == NPC_COILFANG_AMBUSHER && distance < minAmbusherDistance)
        {
            ambusher = unit;
            minAmbusherDistance = distance;
        }
    }

    Unit* target = guardian ? guardian : ambusher;

    if (!target)
    {
        ClearSerpentshrinePriority(botAI);
        return false;
    }

    SetSerpentshrinePriority(botAI, target);

    if (AI_VALUE(Unit*, "current target") != target)
    {
        botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(target);
    }

    bool attacked = Attack(target);
    if (attacked)
    {
        botAI->SetNextCheckDelay(150);
    }

    return attacked;
}

bool LurkerPositionAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    float platformZ = LURKER_PLATFORM_Z;
    if (bot->IsInWater() || bot->GetPositionZ() < platformZ - 0.5f)
    {
        float recoverRadius = botAI->IsMelee(bot) ? 6.5f : 20.0f;
        float angle = Position::NormalizeOrientation(boss->GetOrientation() + M_PI + GetPersonalOffset(bot, 0.12f));
        Position recoverPos = boss->GetPosition();
        recoverPos.m_positionX += cos(angle) * recoverRadius;
        recoverPos.m_positionY += sin(angle) * recoverRadius;
        float targetZ = platformZ;
        bot->UpdateAllowedPositionZ(recoverPos.m_positionX, recoverPos.m_positionY, targetZ);
        recoverPos.m_positionZ = targetZ;

        return MoveTo(bot->GetMapId(), recoverPos.m_positionX, recoverPos.m_positionY, recoverPos.m_positionZ,
                     false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }

    // Don't reposition during spout or when submerged
    if (boss->HasAura(SPELL_LURKER_SPOUT_VISUAL) || boss->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
    {
        return false;
    }

    float desiredDistance = botAI->IsMelee(bot) ? 5.5f : 22.0f;
    float currentDistance = bot->GetDistance(boss);

    // Position properly based on role
    if (fabs(currentDistance - desiredDistance) > 3.0f)
    {
        Position movePos = boss->GetPosition();
        float angle = Position::NormalizeOrientation(boss->GetOrientation() + M_PI + GetPersonalOffset(bot, 0.12f));
        movePos.m_positionX += cos(angle) * desiredDistance;
        movePos.m_positionY += sin(angle) * desiredDistance;
        float targetZ = platformZ;
        bot->UpdateAllowedPositionZ(movePos.m_positionX, movePos.m_positionY, targetZ);
        movePos.m_positionZ = targetZ;

        return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                     false, false, false, false, MovementPriority::MOVEMENT_FORCED);
    }

    return false;
}

// Leotheras the Blind Actions

bool LeotherasWhirlwindAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    Value<uint32>* lastWhirlwindValue = botAI->GetAiObjectContext()->GetValue<uint32>(LEOTHERAS_LAST_WHIRLWIND_TIME);
    Value<uint32>* holdUntilValue = botAI->GetAiObjectContext()->GetValue<uint32>(LEOTHERAS_WHIRLWIND_HOLD_UNTIL);

    bool bossIsWhirling = boss->FindCurrentSpellBySpellId(SPELL_LEOTHERAS_WHIRLWIND) || boss->HasAura(SPELL_LEOTHERAS_WHIRLWIND);
    if (bossIsWhirling)
    {
        uint32 currentTime = getMSTime();

        if (holdUntilValue && !botAI->IsTank(bot))
        {
            holdUntilValue->Set(currentTime + 3000);
        }

        float distance = bot->GetDistance(boss);
        if (distance < 15.0f)
        {
            if (lastWhirlwindValue)
            {
                lastWhirlwindValue->Set(currentTime);
            }

            Position movePos = bot->GetPosition();
            float angle = bot->GetAngle(boss) + M_PI;
            movePos.m_positionX += cos(angle) * 20.0f;
            movePos.m_positionY += sin(angle) * 20.0f;

            return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                         false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
        }

        return false;
    }

    if (lastWhirlwindValue && lastWhirlwindValue->Get() != 0)
    {
        lastWhirlwindValue->Set(0);
        if (holdUntilValue && !botAI->IsTank(bot))
        {
            holdUntilValue->Set(getMSTime() + 3000);
        }
    }

    return false;
}

bool LeotherasChaosBlastAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    Value<float>* spreadValue = botAI->GetAiObjectContext()->GetValue<float>("leotheras chaos blast spread distance");
    if (!spreadValue)
    {
        return false;
    }

    bool shouldSpread = false;

    bool bossCastingChaosBlast = boss->HasAura(SPELL_LEOTHERAS_METAMORPHOSIS) && IsCastingSpell(boss, SPELL_LEOTHERAS_CHAOS_BLAST);

    if (bot->HasAura(SPELL_LEOTHERAS_CHAOS_BLAST))
    {
        shouldSpread = true;
    }
    else if (bossCastingChaosBlast)
    {
        Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
        if (membersValue)
        {
            GuidVector members = membersValue->Get();
            for (ObjectGuid const& member : members)
            {
                Unit* player = botAI->GetUnit(member);
                if (player && player != bot && player->IsAlive())
                {
                    float distance = bot->GetDistance(player);
                    if (distance < 12.0f)
                    {
                        shouldSpread = true;
                        break;
                    }
                }
            }
        }
    }

    float desiredDistance = botAI->IsMelee(bot) ? 9.0f : 18.0f;
    if (shouldSpread)
    {
        if (spreadValue->Get() < desiredDistance)
        {
            spreadValue->Set(desiredDistance);
            UpdateSerpentshrineDisperseDistance(botAI);
        }

        Position movePos = boss->GetPosition();
        float angle = Position::NormalizeOrientation(boss->GetOrientation() + M_PI + GetPersonalOffset(bot, 0.1f));
        movePos.m_positionX += cos(angle) * desiredDistance;
        movePos.m_positionY += sin(angle) * desiredDistance;
        float targetZ = movePos.m_positionZ;
        bot->UpdateAllowedPositionZ(movePos.m_positionX, movePos.m_positionY, targetZ);
        movePos.m_positionZ = targetZ;

        float distanceToMove = bot->GetDistance(movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ);
        if (distanceToMove < 1.5f)
        {
            return false;
        }

        if (MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                   false, false, false, false, MovementPriority::MOVEMENT_NORMAL))
        {
            botAI->SetNextCheckDelay(250);
            return true;
        }

        return false;
    }

    if (spreadValue->Get() > 0.0f)
    {
        spreadValue->Set(0.0f);
        UpdateSerpentshrineDisperseDistance(botAI);
    }

    return false;
}

bool LeotherasInnerDemonAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    // Look for Inner Demon that belongs to this bot
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
    {
        return false;
    }
    GuidVector npcs = npcsValue->Get();
    
    Value<bool>* hasDemonValue = botAI->GetAiObjectContext()->GetValue<bool>(LEOTHERAS_HAS_DEMON);
    Unit* myDemon = nullptr;
    float minDistance = 100.0f;

    for (ObjectGuid const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_INNER_DEMON)
        {
            // Inner Demons can only be damaged by their summoned player
            // Check if this demon is summoned for this bot
            if (unit->ToCreature() && unit->ToCreature()->GetSummonerGUID() == bot->GetGUID())
            {
                float distance = bot->GetDistance(unit);
                if (distance < minDistance)
                {
                    myDemon = unit;
                    minDistance = distance;
                    if (hasDemonValue)
                    {
                        hasDemonValue->Set(true);
                    }
                }
            }
        }
    }

    if (myDemon)
    {
        // Priority target - must kill inner demon quickly
        if (AI_VALUE(Unit*, "current target") != myDemon)
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(myDemon);
        }
        return Attack(myDemon);
    }
    else if (hasDemonValue && hasDemonValue->Get())
    {
        // Demon was killed, clear flag
        hasDemonValue->Set(false);
    }

    return false;
}

bool LeotherasShadowAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    Value<ObjectGuid>* focusValue = botAI->GetAiObjectContext()->GetValue<ObjectGuid>(LEOTHERAS_SHADOW_TARGET);

    if (boss->GetHealthPct() > 15.0f)
    {
        if (focusValue && !focusValue->Get().IsEmpty())
            focusValue->Set(ObjectGuid::Empty);
        ClearSerpentshrinePriority(botAI);
        return false;
    }

    if (!PlayerbotAI::IsRangedDps(bot))
    {
        if (focusValue && !focusValue->Get().IsEmpty())
            focusValue->Set(ObjectGuid::Empty);
        ClearSerpentshrinePriority(botAI);
        return false;
    }

    Unit* shadow = ResolveLeotherasShadow(botAI, bot, focusValue);
    if (!shadow)
    {
        ClearSerpentshrinePriority(botAI);
        return false;
    }

    SetSerpentshrinePriority(botAI, shadow);

    if (AI_VALUE(Unit*, "current target") != shadow)
    {
        botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(shadow);
    }

    bool attacked = Attack(shadow);
    if (attacked)
    {
        botAI->SetNextCheckDelay(150);
    }

    return attacked;
}

bool LeotherasPositionAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Don't reposition during whirlwind or while actively spreading
    if (boss->HasAura(SPELL_LEOTHERAS_WHIRLWIND) || AI_VALUE(float, "leotheras chaos blast spread distance") > 0.0f)
    {
        return false;
    }

    Unit* anchor = boss;
    bool anchorIsShadow = false;

    bool const isRangedDps = PlayerbotAI::IsRangedDps(bot);

    if (boss->GetHealthPct() <= 15.0f && isRangedDps)
    {
        Value<ObjectGuid>* focusValue = botAI->GetAiObjectContext()->GetValue<ObjectGuid>(LEOTHERAS_SHADOW_TARGET);
        if (focusValue && !focusValue->Get().IsEmpty())
        {
            Unit* focusShadow = botAI->GetUnit(focusValue->Get());
            if (focusShadow && focusShadow->IsAlive())
            {
                anchor = focusShadow;
                anchorIsShadow = true;
            }
        }

        if (!anchorIsShadow)
        {
            Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
            if (npcsValue)
            {
                Unit* closestShadow = nullptr;
                float minDistance = 100.0f;
                for (ObjectGuid const& guid : npcsValue->Get())
                {
                    Unit* npc = botAI->GetUnit(guid);
                    if (!npc || !npc->IsAlive() || npc->GetEntry() != NPC_SHADOW_OF_LEOTHERAS)
                        continue;

                    float distance = bot->GetDistance(npc);
                    if (distance < minDistance)
                    {
                        closestShadow = npc;
                        minDistance = distance;
                    }
                }

                if (closestShadow)
                {
                    anchor = closestShadow;
                    anchorIsShadow = true;
                    if (focusValue)
                    {
                        focusValue->Set(anchor->GetGUID());
                    }
                }
            }
        }
    }

    float desiredDistance;
    if (anchorIsShadow)
    {
        desiredDistance = botAI->IsRanged(bot) ? 22.0f : 5.0f;
    }
    else if (boss->HasAura(SPELL_LEOTHERAS_METAMORPHOSIS))
    {
        desiredDistance = botAI->IsMelee(bot) ? 6.0f : 22.0f;
    }
    else
    {
        desiredDistance = botAI->IsMelee(bot) ? 5.0f : 25.0f;
    }

    Position movePos = anchor->GetPosition();
    float angle = anchor->GetAngle(bot);
    movePos.m_positionX += cos(angle) * desiredDistance;
    movePos.m_positionY += sin(angle) * desiredDistance;
    float targetZ = movePos.m_positionZ;
    bot->UpdateAllowedPositionZ(movePos.m_positionX, movePos.m_positionY, targetZ);
    movePos.m_positionZ = targetZ;

    float travelDistance = bot->GetDistance(movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ);
    if (travelDistance < 1.5f)
    {
        return false;
    }

    return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                  false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
}

// Fathom-Lord Karathress Actions
bool KarathressCataclysmicBoltAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Cataclysmic Bolt targets mana users - move away to reduce damage
    if (bot->getPowerType() == POWER_MANA && boss->FindCurrentSpellBySpellId(SPELL_KARATHRESS_CATACLYSMIC_BOLT))
    {
        float distance = bot->GetDistance(boss);
        if (distance < 30.0f) // Try to outrange if possible
        {
            Position movePos = bot->GetPosition();
            float angle = bot->GetAngle(boss) + M_PI;
            movePos.m_positionX += cos(angle) * 10.0f;
            movePos.m_positionY += sin(angle) * 10.0f;
            
            return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                         false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
        }
    }

    return false;
}

bool KarathressSearNovaAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        if (Value<float>* spreadValue = botAI->GetAiObjectContext()->GetValue<float>("karathress sear nova spread distance"))
        {
            if (spreadValue->Get() > 0.0f)
            {
                spreadValue->Set(0.0f);
                UpdateSerpentshrineDisperseDistance(botAI);
            }
        }
        return false;
    }

    Value<float>* spreadValue = botAI->GetAiObjectContext()->GetValue<float>("karathress sear nova spread distance");

    // Sear Nova is a melee AoE - spread out
    if (boss->FindCurrentSpellBySpellId(SPELL_KARATHRESS_SEAR_NOVA) || 
        (boss->HasUnitState(UNIT_STATE_CASTING) && bot->GetDistance(boss) < 10.0f))
    {
        if (spreadValue)
        {
            constexpr float desiredDistance = 15.0f;
            if (spreadValue->Get() < desiredDistance)
            {
                spreadValue->Set(desiredDistance);
                UpdateSerpentshrineDisperseDistance(botAI);
            }
        }
        botAI->SetNextCheckDelay(250);
        return true;
    }

    if (spreadValue && spreadValue->Get() > 0.0f)
    {
        spreadValue->Set(0.0f);
        UpdateSerpentshrineDisperseDistance(botAI);
    }

    return false;
}

bool KarathressAdvisorsAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        ClearSerpentshrinePriority(botAI);
        return false;
    }

    auto const hasActiveTotem = [&]() -> bool
    {
        Value<ObjectGuid>* storedTotem = botAI->GetAiObjectContext()->GetValue<ObjectGuid>(KARATHRESS_TOTEM_TARGET);
        if (storedTotem && !storedTotem->Get().IsEmpty())
        {
            if (Unit* tracked = botAI->GetUnit(storedTotem->Get()))
            {
                if (tracked->IsAlive())
                {
                    return true;
                }
            }
        }

        auto containsTotem = [&](char const* contextName) -> bool
        {
            Value<GuidVector>* vecValue = botAI->GetAiObjectContext()->GetValue<GuidVector>(contextName);
            if (!vecValue)
                return false;

            for (ObjectGuid const& guid : vecValue->Get())
            {
                Unit* unit = botAI->GetUnit(guid);
                if (!unit || !unit->IsAlive())
                    continue;

                switch (unit->GetEntry())
                {
                    case NPC_SPITFIRE_TOTEM:
                    case NPC_GREATER_EARTHBIND_TOTEM:
                    case NPC_GREATER_POISON_CLEANSING_TOTEM:
                        return true;
                    default:
                        break;
                }
            }

            return false;
        };

        if (containsTotem("possible targets"))
            return true;
        if (containsTotem("possible targets no los"))
            return true;
        if (containsTotem("nearest hostile npcs"))
            return true;

        return false;
    };

    if (hasActiveTotem())
    {
        return false;
    }

    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
    {
        return false;
    }
    GuidVector npcs = npcsValue->Get();
    
    // Kill order: Tidalvess (shaman) -> Sharkkis (hunter) -> Caribdis (priest) -> Karathress
    Unit* tidalvess = nullptr;
    Unit* sharkkis = nullptr;
    Unit* caribdis = nullptr;
    
    float minTidalvessDistance = 100.0f;
    float minSharkkisDistance = 100.0f;
    float minCaribdisDistance = 100.0f;

    for (ObjectGuid const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || !unit->IsAlive())
            continue;

        float distance = bot->GetDistance(unit);
        
        if (unit->GetEntry() == NPC_FATHOM_GUARD_TIDALVESS && distance < minTidalvessDistance)
        {
            tidalvess = unit;
            minTidalvessDistance = distance;
        }
        else if (unit->GetEntry() == NPC_FATHOM_GUARD_SHARKKIS && distance < minSharkkisDistance)
        {
            sharkkis = unit;
            minSharkkisDistance = distance;
        }
        else if (unit->GetEntry() == NPC_FATHOM_GUARD_CARIBDIS && distance < minCaribdisDistance)
        {
            caribdis = unit;
            minCaribdisDistance = distance;
        }
    }

    // Priority order - kill Tidalvess first (totems), then Sharkkis (pets), then Caribdis (healer)
    Unit* target = nullptr;
    if (tidalvess)
    {
        target = tidalvess;
    }
    else if (sharkkis)
    {
        target = sharkkis;
    }
    else if (caribdis)
    {
        target = caribdis;
    }

    if (target)
    {
        if (AI_VALUE(Unit*, "current target") != target)
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(target);
        }
        SetSerpentshrinePriority(botAI, target);
        return Attack(target);
    }

    ClearSerpentshrinePriority(botAI);
    return false;
}

bool KarathressTidalSurgeAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    // Check for Caribdis casting Tidal Surge
    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
    {
        return false;
    }
    GuidVector npcs = npcsValue->Get();
    
    for (ObjectGuid const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_FATHOM_GUARD_CARIBDIS)
        {
            if (unit->FindCurrentSpellBySpellId(SPELL_KARATHRESS_TIDAL_SURGE))
            {
                float distance = bot->GetDistance(unit);
                if (distance < 10.0f) // Tidal Surge is a frontal cone stun
                {
                    Position movePos = bot->GetPosition();
                    float angle = bot->GetAngle(unit) + M_PI / 2; // Move to the side
                    movePos.m_positionX += cos(angle) * 8.0f;
                    movePos.m_positionY += sin(angle) * 8.0f;
                    
                    return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                                 false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
                }
            }
        }
    }

    return false;
}

bool KarathressCycloneAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    // WAIT STATE: Bot is cycloned, let the spell handle positioning
    // Just return true to prevent other movement actions from interfering
    return true; // ICC Pattern: return true for wait states
}

bool KarathressSpreadAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    // WotLK Standard Pattern: Use disperse distance AI value system
    // 12 yards provides good spread to prevent cyclone from hitting multiple ranged
    SET_AI_VALUE(float, "disperse distance", 12.0f);

    return true;
}

bool KarathressCycloneFallAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    // Force natural falling by clearing motion and updating position to ground
    bot->GetMotionMaster()->Clear();

    float currentZ = bot->GetPositionZ();
    float groundZ = bot->GetMapWaterOrGroundLevel(bot->GetPositionX(), bot->GetPositionY(), currentZ);

    // Immediately teleport to ground to simulate natural fall completion
    bot->NearTeleportTo(bot->GetPositionX(), bot->GetPositionY(), groundZ, bot->GetOrientation());

    return true;
}

bool KarathressTotemsAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    // UNIVERSAL DEBUG: Prove this action can execute in Serpentshrine
    static std::map<ObjectGuid, uint32> g_universalDebugTime;
    uint32 currentTime = getMSTime();

    if (bot->GetMapId() == 548 && g_universalDebugTime[bot->GetGUID()] + 5000 < currentTime)
    {
        g_universalDebugTime[bot->GetGUID()] = currentTime;
        LOG_INFO("playerbots", "UNIVERSAL_DEBUG: {} | Totems action executing in Serpentshrine - strategy IS working",
            bot->GetName().c_str());
    }

    if (botAI->IsHeal(bot))
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        if (Value<ObjectGuid>* stored = botAI->GetAiObjectContext()->GetValue<ObjectGuid>(KARATHRESS_TOTEM_TARGET))
        {
            if (!stored->Get().IsEmpty())
            {
                stored->Set(ObjectGuid::Empty);
            }
        }
        return false;
    }

    Value<ObjectGuid>* storedTotemValue = botAI->GetAiObjectContext()->GetValue<ObjectGuid>(KARATHRESS_TOTEM_TARGET);
    Unit* target = nullptr;

    if (storedTotemValue && !storedTotemValue->Get().IsEmpty())
    {
        target = botAI->GetUnit(storedTotemValue->Get());
        if (!target || !target->IsAlive())
        {
            target = nullptr;
            storedTotemValue->Set(ObjectGuid::Empty);
        }
    }

    auto findPriorityTotem = [&](GuidVector const& candidates)
    {
        Unit* spitfire = nullptr;
        Unit* earthbind = nullptr;
        Unit* cleansing = nullptr;

        for (ObjectGuid const& npcGuid : candidates)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || !unit->IsAlive())
                continue;

            switch (unit->GetEntry())
            {
                case NPC_SPITFIRE_TOTEM:
                    spitfire = unit;
                    break;
                case NPC_GREATER_EARTHBIND_TOTEM:
                    earthbind = unit;
                    break;
                case NPC_GREATER_POISON_CLEANSING_TOTEM:
                    cleansing = unit;
                    break;
                default:
                    break;
            }

            if (spitfire)
                break;
        }

        if (spitfire)
            return spitfire;
        if (earthbind)
            return earthbind;
        if (cleansing)
            return cleansing;

        return static_cast<Unit*>(nullptr);
    };

    if (!target)
    {
        if (Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets"))
        {
            target = findPriorityTotem(npcsValue->Get());
        }

        if (!target)
        {
            if (Value<GuidVector>* npcsNoLosValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los"))
            {
                target = findPriorityTotem(npcsNoLosValue->Get());
            }
        }

        if (target && storedTotemValue)
        {
            storedTotemValue->Set(target->GetGUID());
        }
    }

    if (!target)
    {
        ClearSerpentshrinePriority(botAI);
        return false;
    }

    if (AI_VALUE(Unit*, "current target") != target)
    {
        botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(target);
    }

    SetSerpentshrinePriority(botAI, target);

    bool hasLos = bot->IsWithinLOSInMap(target);
    float requiredRange = PlayerbotAI::IsRanged(bot) ? 28.0f : 4.5f;
    if (!hasLos || !bot->IsWithinDistInMap(target, requiredRange))
    {
        Position const& pos = target->GetPosition();
        botAI->SetNextCheckDelay(150);
        return MoveTo(target->GetMapId(), pos.m_positionX, pos.m_positionY, pos.m_positionZ,
                      false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
    }

    bool attacked = Attack(target);
    if (attacked)
    {
        botAI->SetNextCheckDelay(150);
        return true;
    }

    botAI->SetNextCheckDelay(200);
    return false;
}

// Morogrim Tidewalker Actions

bool MorogrimTidalWaveAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Check if boss is casting Tidal Wave and bot is in front
    if (boss->FindCurrentSpellBySpellId(SPELL_MOROGRIM_TIDAL_WAVE) || boss->HasUnitState(UNIT_STATE_CASTING))
    {
        // Check if bot is in front of boss
        float angle = boss->GetAngle(bot);
        float facing = boss->GetOrientation();
        float diff = fabs(angle - facing);
        
        // Normalize angle difference
        if (diff > M_PI)
            diff = 2 * M_PI - diff;
            
        // In front cone (90 degrees)
        if (diff < M_PI / 4)
        {
            float distance = bot->GetDistance(boss);
            if (distance < 20.0f)
            {
                // Move behind boss
                Position movePos = boss->GetPosition();
                float moveAngle = facing + M_PI; // Behind boss
                movePos.m_positionX += cos(moveAngle) * 8.0f;
                movePos.m_positionY += sin(moveAngle) * 8.0f;
                
                return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                             false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
            }
        }
    }

    return false;
}

bool MorogrimWateryGraveAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    // Check if bot has Watery Grave debuff
    if (bot->HasAura(SPELL_MOROGRIM_WATERY_GRAVE_1) || 
        bot->HasAura(SPELL_MOROGRIM_WATERY_GRAVE_2) || 
        bot->HasAura(SPELL_MOROGRIM_WATERY_GRAVE_3) || 
        bot->HasAura(SPELL_MOROGRIM_WATERY_GRAVE_4))
    {
        uint32 currentTime = getMSTime();
        if (Value<uint32>* graveTimeValue = botAI->GetAiObjectContext()->GetValue<uint32>(MOROGRIM_LAST_GRAVE_TIME))
        {
            graveTimeValue->Set(currentTime);
        }
        
        // Use healthstone if available and health is low
        if (bot->GetHealthPct() < 50.0f)
        {
            // This would trigger healthstone usage through normal healing logic
            return true;
        }
    }

    return false;
}

bool MorogrimMurlocsAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
    if (!npcsValue)
    {
        return false;
    }
    GuidVector npcs = npcsValue->Get();
    
    Unit* closestMurloc = nullptr;
    float minDistance = 100.0f;

    for (ObjectGuid const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || !unit->IsAlive())
            continue;

        // Tidewalker Lurker murlocs
        if (unit->GetEntry() == NPC_TIDEWALKER_LURKER)
        {
            float distance = bot->GetDistance(unit);
            if (distance < minDistance)
            {
                closestMurloc = unit;
                minDistance = distance;
            }
        }
    }

    if (closestMurloc)
    {
        // Switch to murlocs for AOE
        if (AI_VALUE(Unit*, "current target") != closestMurloc)
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(closestMurloc);
        }
        
        // Move to stack point for AOE if tank
        if (botAI->IsTank(bot))
        {
            Unit* boss = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
            if (boss)
            {
                // Bring murlocs to boss for cleave
                Position stackPos = boss->GetPosition();
                float distance = bot->GetDistance(stackPos);
                if (distance > 5.0f)
                {
                    return MoveTo(bot->GetMapId(), stackPos.m_positionX, stackPos.m_positionY, stackPos.m_positionZ,
                                 false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
                }
            }
        }
        
        return Attack(closestMurloc);
    }

    return false;
}

bool MorogrimGlobulesAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Only in phase 2 (below 25%)
    if (boss->GetHealthPct() <= 25.0f)
    {
        Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
        if (!npcsValue)
        {
            return false;
        }
        GuidVector npcs = npcsValue->Get();
        
        Unit* closestGlobule = nullptr;
        float minDistance = 100.0f;

        for (ObjectGuid const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || !unit->IsAlive())
                continue;

            // Water Globule
            if (unit->GetEntry() == NPC_WATER_GLOBULE)
            {
                float distance = bot->GetDistance(unit);
                if (distance < minDistance)
                {
                    closestGlobule = unit;
                    minDistance = distance;
                }
            }
        }

        if (closestGlobule && minDistance < 15.0f)
        {
            // Kite away from globule
            Position movePos = bot->GetPosition();
            float angle = bot->GetAngle(closestGlobule) + M_PI;
            movePos.m_positionX += cos(angle) * 10.0f;
            movePos.m_positionY += sin(angle) * 10.0f;
            
            return MoveTo(bot->GetMapId(), movePos.m_positionX, movePos.m_positionY, movePos.m_positionZ,
                         false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
        }
    }

    return false;
}

bool MorogrimPositionAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    float desiredDistance;
    Position desiredPos;
    
    if (boss->GetHealthPct() <= 25.0f)
    {
        // Phase 2 - move to entrance, need room for globules
        desiredDistance = botAI->IsMelee(bot) ? 5.0f : 30.0f;
        
        // Position at entrance
        desiredPos = boss->GetPosition();
    }
    else
    {
        // Phase 1 - stack behind boss
        desiredDistance = botAI->IsMelee(bot) ? 5.0f : 15.0f;
        
        // Get behind boss
        float bossOrientation = boss->GetOrientation();
        desiredPos = boss->GetPosition();
        desiredPos.m_positionX += cos(bossOrientation + M_PI) * desiredDistance;
        desiredPos.m_positionY += sin(bossOrientation + M_PI) * desiredDistance;
    }

    float currentDistance = bot->GetDistance(desiredPos);
    if (currentDistance > 5.0f)
    {
        return MoveTo(bot->GetMapId(), desiredPos.m_positionX, desiredPos.m_positionY, desiredPos.m_positionZ,
                     false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
    }

    return false;
}

// Lady Vashj Actions
bool VashjShockBlastAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Check if boss is casting Shock Blast (frontal cone)
    if (boss->FindCurrentSpellBySpellId(SPELL_VASHJ_SHOCK_BLAST))
    {
        // Get behind or to the side of boss
        float angle = boss->GetOrientation() + M_PI;
        float distance = 10.0f;
        float x = boss->GetPositionX() + cos(angle) * distance;
        float y = boss->GetPositionY() + sin(angle) * distance;
        float z = boss->GetPositionZ();
        
        boss->UpdateAllowedPositionZ(x, y, z);
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
    }

    return false;
}

bool VashjStaticChargeAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Value<float>* spreadValue = botAI->GetAiObjectContext()->GetValue<float>("vashj static charge spread distance");
    if (!spreadValue)
    {
        return false;
    }

    if (bot->HasAura(SPELL_VASHJ_STATIC_CHARGE))
    {
        Value<GuidVector>* membersValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("group members");
        if (membersValue)
        {
            GuidVector members = membersValue->Get();
            for (ObjectGuid const& member : members)
            {
                Unit* player = botAI->GetUnit(member);
                if (player && player != bot && player->IsAlive() && bot->GetDistance(player) < 10.0f)
                {
                    constexpr float desiredDistance = 15.0f;
                    if (spreadValue->Get() < desiredDistance)
                    {
                        spreadValue->Set(desiredDistance);
                        UpdateSerpentshrineDisperseDistance(botAI);
                    }
                    botAI->SetNextCheckDelay(250);
                    return true;
                }
            }
        }
    }

    if (spreadValue->Get() > 0.0f)
    {
        spreadValue->Set(0.0f);
        UpdateSerpentshrineDisperseDistance(botAI);
    }

    return false;
}

bool VashjEntangleAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    // Check if entangled and need dispel/freedom
    if (bot->HasAura(SPELL_VASHJ_ENTANGLE))
    {
        // Request dispel or use freedom abilities
        Value<std::list<uint32>>* dispelSpellsValue = botAI->GetAiObjectContext()->GetValue<std::list<uint32>>("dispel spell", "root");
        if (dispelSpellsValue)
        {
            std::list<uint32> dispelSpells = dispelSpellsValue->Get();
            for (uint32 spellId : dispelSpells)
            {
                if (botAI->CanCastSpell(spellId, bot))
                {
                    return botAI->CastSpell(spellId, bot);
                }
            }
        }
    }

    return false;
}

bool VashjEnchantedElementalAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Phase 2 - Enchanted Elementals must die quickly
    if (boss->HasAura(SPELL_VASHJ_MAGIC_BARRIER))
    {
        Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
        if (!npcsValue)
        {
            return false;
        }
        GuidVector npcs = npcsValue->Get();
        
        Unit* nearestElemental = nullptr;
        float minDistance = 100.0f;
        
        for (ObjectGuid const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->GetEntry() == NPC_ENCHANTED_ELEMENTAL)
            {
                float distance = bot->GetDistance(unit);
                if (distance < minDistance)
                {
                    minDistance = distance;
                    nearestElemental = unit;
                }
            }
        }
        
        if (nearestElemental)
        {
            if (AI_VALUE(Unit*, "current target") != nearestElemental)
            {
                botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nearestElemental);
            }
            return Attack(nearestElemental);
        }
    }

    return false;
}

bool VashjTaintedElementalAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Phase 2 - Tainted Elementals drop cores needed for shield generators
    if (boss->HasAura(SPELL_VASHJ_MAGIC_BARRIER))
    {
        Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
        if (!npcsValue)
        {
            return false;
        }
        GuidVector npcs = npcsValue->Get();
        
        Unit* nearestTainted = nullptr;
        float minDistance = 100.0f;
        
        for (ObjectGuid const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->GetEntry() == NPC_TAINTED_ELEMENTAL)
            {
                float distance = bot->GetDistance(unit);
                if (distance < minDistance)
                {
                    minDistance = distance;
                    nearestTainted = unit;
                }
            }
        }
        
        if (nearestTainted)
        {
            if (AI_VALUE(Unit*, "current target") != nearestTainted)
            {
                botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nearestTainted);
            }
            return Attack(nearestTainted);
        }
    }

    return false;
}

bool VashjCoilfangEliteAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Phase 2 - Coilfang Elites are high priority
    if (boss->HasAura(SPELL_VASHJ_MAGIC_BARRIER))
    {
        Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
        if (!npcsValue)
        {
            return false;
        }
        GuidVector npcs = npcsValue->Get();
        
        Unit* nearestElite = nullptr;
        float minDistance = 100.0f;
        
        for (ObjectGuid const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->GetEntry() == NPC_COILFANG_ELITE)
            {
                float distance = bot->GetDistance(unit);
                if (distance < minDistance)
                {
                    minDistance = distance;
                    nearestElite = unit;
                }
            }
        }
        
        if (nearestElite)
        {
            if (AI_VALUE(Unit*, "current target") != nearestElite)
            {
                botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nearestElite);
            }
            return Attack(nearestElite);
        }
    }

    return false;
}

bool VashjCoilfangStriderAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Phase 2 - Coilfang Striders fear and must be controlled
    if (boss->HasAura(SPELL_VASHJ_MAGIC_BARRIER))
    {
        Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
        if (!npcsValue)
        {
            return false;
        }
        GuidVector npcs = npcsValue->Get();
        
        Unit* nearestStrider = nullptr;
        float minDistance = 100.0f;
        
        for (ObjectGuid const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->GetEntry() == NPC_COILFANG_STRIDER)
            {
                float distance = bot->GetDistance(unit);
                if (distance < minDistance)
                {
                    minDistance = distance;
                    nearestStrider = unit;
                }
            }
        }
        
        if (nearestStrider)
        {
            // Kite striders if possible
            if (botAI->IsRanged(bot) && minDistance < 15.0f)
            {
                float angle = bot->GetAngle(nearestStrider) + M_PI;
                float moveDistance = 20.0f;
                float x = bot->GetPositionX() + cos(angle) * moveDistance;
                float y = bot->GetPositionY() + sin(angle) * moveDistance;
                float z = bot->GetPositionZ();
                
                bot->UpdateAllowedPositionZ(x, y, z);
                return MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
            }
            
            if (AI_VALUE(Unit*, "current target") != nearestStrider)
            {
                botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nearestStrider);
            }
            return Attack(nearestStrider);
        }
    }

    return false;
}

bool VashjSporebatAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Phase 3 - Toxic Sporebats drop poison clouds
    if (!boss->HasAura(SPELL_VASHJ_MAGIC_BARRIER) && boss->GetHealthPct() <= 50.0f)
    {
        Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs");
        if (!npcsValue)
        {
            return false;
        }
        GuidVector npcs = npcsValue->Get();
        
        Unit* nearestBat = nullptr;
        float minDistance = 100.0f;
        
        for (ObjectGuid const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->GetEntry() == NPC_TOXIC_SPOREBAT)
            {
                float distance = bot->GetDistance(unit);
                if (distance < minDistance)
                {
                    minDistance = distance;
                    nearestBat = unit;
                }
            }
        }
        
        if (nearestBat)
        {
            // Ranged should prioritize bats
            if (botAI->IsRanged(bot))
            {
                if (AI_VALUE(Unit*, "current target") != nearestBat)
                {
                    botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nearestBat);
                }
                return Attack(nearestBat);
            }
            // Melee avoid getting close to bats
            else if (minDistance < 10.0f)
            {
                float angle = bot->GetAngle(nearestBat) + M_PI;
                float moveDistance = 15.0f;
                float x = bot->GetPositionX() + cos(angle) * moveDistance;
                float y = bot->GetPositionY() + sin(angle) * moveDistance;
                float z = bot->GetPositionZ();
                
                bot->UpdateAllowedPositionZ(x, y, z);
                return MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
            }
        }
    }

    return false;
}

bool VashjPositionAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    float desiredDistance;
    
    // Phase 2 - spread out for adds
    if (boss->HasAura(SPELL_VASHJ_MAGIC_BARRIER))
    {
        desiredDistance = botAI->IsRanged(bot) ? 30.0f : 20.0f;
    }
    // Phase 1 and 3 - standard positioning
    else
    {
        desiredDistance = botAI->IsMelee(bot) ? 5.0f : 25.0f;
    }
    
    float currentDistance = bot->GetDistance(boss);
    if (fabs(currentDistance - desiredDistance) > 5.0f)
    {
        float angle = boss->GetAngle(bot);
        float x = boss->GetPositionX() + cos(angle) * desiredDistance;
        float y = boss->GetPositionY() + sin(angle) * desiredDistance;
        float z = boss->GetPositionZ();
        
        boss->UpdateAllowedPositionZ(x, y, z);
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
    }

    return false;
}

bool VashjTaintedCoreAction::Execute(Event event)
{
    if (!bot || !botAI)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    // Phase 2 - check if bot has tainted core item
    if (!boss->HasAura(SPELL_VASHJ_MAGIC_BARRIER))
    {
        return false;
    }

    Item* coreItem = bot->GetItemByEntry(ITEM_TAINTED_CORE);
    if (!coreItem)
    {
        return false;
    }

    GameObject* nearestGenerator = nullptr;
    float bestDistance = std::numeric_limits<float>::max();

    for (uint32 entry : s_vashjShieldGenerators)
    {
        if (GameObject* go = bot->FindNearestGameObject(entry, 120.0f, true))
        {
            if (!go->isSpawned())
                continue;

            float distance = bot->GetExactDist(go);
            if (distance < bestDistance)
            {
                bestDistance = distance;
                nearestGenerator = go;
            }
        }
    }

    Position targetPos;
    if (nearestGenerator)
    {
        targetPos = nearestGenerator->GetPosition();
    }
    else
    {
        // Fallback to static spawn coordinates if the GO lookup failed (e.g., out of range)
        for (Position const& pos : s_vashjShieldGeneratorPositions)
        {
            float distance = bot->GetDistance(pos);
            if (distance < bestDistance)
            {
                bestDistance = distance;
                targetPos = pos;
            }
        }

        if (bestDistance == std::numeric_limits<float>::max())
        {
            return false;
        }
    }

    if (bot->GetDistance(targetPos) > 3.0f)
    {
        float adjustedZ = targetPos.GetPositionZ();
        bot->UpdateAllowedPositionZ(targetPos.GetPositionX(), targetPos.GetPositionY(), adjustedZ);
        targetPos.m_positionZ = adjustedZ;
        return MoveTo(bot->GetMapId(), targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ(),
                     false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
    }

    if (!nearestGenerator)
    {
        return false;
    }

    TaintedCoreUseAction useCore(botAI);
    if (useCore.UseItemOnGameObject(coreItem, nearestGenerator->GetGUID()))
    {
        botAI->SetNextCheckDelay(500);
        return true;
    }

    return false;
}
