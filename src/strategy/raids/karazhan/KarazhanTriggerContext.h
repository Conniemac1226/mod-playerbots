#ifndef _PLAYERBOT_KARAZHANTRIGGERCONTEXT_H
#define _PLAYERBOT_KARAZHANTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "KarazhanTriggers.h"

class KarazhanTriggerContext : public NamedObjectContext<Trigger>
{
public:
    KarazhanTriggerContext()
    {
        creators["attumen engaged"] = &KarazhanTriggerContext::attumen_engaged;
        creators["attumen mounted"] = &KarazhanTriggerContext::attumen_mounted;
        creators["attumen charge danger"] = &KarazhanTriggerContext::attumen_charge_danger;
        creators["attumen shadowcleave"] = &KarazhanTriggerContext::attumen_shadowcleave;
    }

private:
    static Trigger* attumen_engaged(PlayerbotAI* ai) { return new AttumenEngagedTrigger(ai); }
    static Trigger* attumen_mounted(PlayerbotAI* ai) { return new AttumenMountedTrigger(ai); }
    static Trigger* attumen_charge_danger(PlayerbotAI* ai) { return new AttumenChargeDangerTrigger(ai); }
    static Trigger* attumen_shadowcleave(PlayerbotAI* ai) { return new AttumenShadowcleaveTrigger(ai); }
};

#endif