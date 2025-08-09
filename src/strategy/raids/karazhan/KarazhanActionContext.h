#ifndef _PLAYERBOT_KARAZHANACTIONCONTEXT_H
#define _PLAYERBOT_KARAZHANACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "KarazhanActions.h"

class KarazhanActionContext : public NamedObjectContext<Action>
{
public:
    KarazhanActionContext()
    {
        creators["attumen avoid charge"] = &KarazhanActionContext::attumen_avoid_charge;
        creators["attumen position"] = &KarazhanActionContext::attumen_position;
    }

private:
    static Action* attumen_avoid_charge(PlayerbotAI* ai) { return new AttumenAvoidChargeAction(ai); }
    static Action* attumen_position(PlayerbotAI* ai) { return new AttumenPositionAction(ai); }
};

#endif