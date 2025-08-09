#ifndef _PLAYERBOT_TBCDUNGEONSHACTIONCONTEXT_H
#define _PLAYERBOT_TBCDUNGEONSHACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "SethekkHallsActions.h"

class TbcDungeonSHActionContext : public NamedObjectContext<Action>
{
    public:
        TbcDungeonSHActionContext() {
            creators["attack charming totem"] = &TbcDungeonSHActionContext::attack_charming_totem;
            creators["interrupt controller"] = &TbcDungeonSHActionContext::interrupt_controller;
            creators["ikiss move away"] = &TbcDungeonSHActionContext::ikiss_move_away;
            creators["ikiss return position"] = &TbcDungeonSHActionContext::ikiss_return_position;
        }
    private:
        static Action* attack_charming_totem(PlayerbotAI* ai) { return new AttackCharmingTotemAction(ai); }
        static Action* interrupt_controller(PlayerbotAI* ai) { return new InterruptControllerAction(ai); }
        static Action* ikiss_move_away(PlayerbotAI* ai) { return new IkissMoveAwayAction(ai); }
        static Action* ikiss_return_position(PlayerbotAI* ai) { return new IkissReturnPositionAction(ai); }
};

#endif