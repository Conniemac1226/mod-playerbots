
#include "ChangeTargetAction.h"
#include "../PlayerbotAI.h"
#include "Playerbots.h"

using namespace ai;

bool ChangeTargetAction::Execute(Event event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    ObjectGuid newTargetGuid = event.getObject();

    if (!newTargetGuid)
    {
        return false;
    }

    bot->SetSelection(newTargetGuid);
    return true;
}

bool ChangeTargetAction::isUseful()
{
    return true;
}
