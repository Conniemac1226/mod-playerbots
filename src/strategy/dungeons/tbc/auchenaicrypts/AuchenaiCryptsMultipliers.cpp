#include "AuchenaiCryptsMultipliers.h"
#include "AuchenaiCryptsTriggers.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"
#include "AttackersValue.h"

float ShirrakFocusFireMultiplier::GetValue(Action* action)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return 1.0f;

    // Critical priority for Focus Fire avoidance
    if (action->getName() == "shirrak focus fire avoid")
    {
        // Check if Focus Fire creature exists
        std::list<Unit*> targets;
        Acore::AnyUnitInObjectRangeCheck u_check(bot, 60.0f);
        Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
        Cell::VisitObjects(bot, searcher, 60.0f);

        for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
        {
            Unit* unit = *i;
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->GetEntry() == NPC_FOCUS_FIRE)
            {
                return 5.0f;
            }
        }
    }

    return 1.0f;
}