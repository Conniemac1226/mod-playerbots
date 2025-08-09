#ifndef _PLAYERBOT_TBCDUNGEONSACACTIONCONTEXT_H
#define _PLAYERBOT_TBCDUNGEONSACACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "AuchenaiCryptsActions.h"

class TbcDungeonACActionContext : public NamedObjectContext<Action>
{
public:
    TbcDungeonACActionContext()
    {
        creators["shirrak focus fire avoid"] = &TbcDungeonACActionContext::shirrak_focus_fire_avoid;
        creators["shirrak return position"] = &TbcDungeonACActionContext::shirrak_return_position;
    }

private:
    static Action* shirrak_focus_fire_avoid(PlayerbotAI* ai) { return new ShirrakFocusFireAvoidAction(ai); }
    static Action* shirrak_return_position(PlayerbotAI* ai) { return new ShirrakReturnPositionAction(ai); }
};

#endif