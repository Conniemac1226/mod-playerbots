#ifndef _PLAYERBOT_MANATOMBSTRIGGERCONTEXT_H
#define _PLAYERBOT_MANATOMBSTRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "ManaTombsTriggers.h"

class ManaTombsTriggerContext : public NamedObjectContext<Trigger>
{
public:
    ManaTombsTriggerContext()
    {
        creators["dark shell active"] = &ManaTombsTriggerContext::dark_shell_active;
        creators["void blast spread"] = &ManaTombsTriggerContext::void_blast_spread;
        creators["earthquake casting"] = &ManaTombsTriggerContext::earthquake_casting;
        creators["crystal prison active"] = &ManaTombsTriggerContext::crystal_prison_active;
        creators["arcing smash danger"] = &ManaTombsTriggerContext::arcing_smash_danger;
        creators["ethereal beacon active"] = &ManaTombsTriggerContext::ethereal_beacon_active;
        creators["frost nova danger"] = &ManaTombsTriggerContext::frost_nova_danger;
        creators["shaffar blinked"] = &ManaTombsTriggerContext::shaffar_blinked;
        creators["double breath danger"] = &ManaTombsTriggerContext::double_breath_danger;
        creators["stomp danger"] = &ManaTombsTriggerContext::stomp_danger;
    }

private:
    static Trigger* dark_shell_active(PlayerbotAI* ai) { return new PandemoniusDarkShellTrigger(ai); }
    static Trigger* void_blast_spread(PlayerbotAI* ai) { return new PandemoniusVoidBlastTrigger(ai); }
    static Trigger* earthquake_casting(PlayerbotAI* ai) { return new TavarokEarthquakeTrigger(ai); }
    static Trigger* crystal_prison_active(PlayerbotAI* ai) { return new TavarokCrystalPrisonTrigger(ai); }
    static Trigger* arcing_smash_danger(PlayerbotAI* ai) { return new TavarokArcingSmashTrigger(ai); }
    static Trigger* ethereal_beacon_active(PlayerbotAI* ai) { return new EtherealBeaconActiveTrigger(ai); }
    static Trigger* frost_nova_danger(PlayerbotAI* ai) { return new ShaffarFrostNovaTrigger(ai); }
    static Trigger* shaffar_blinked(PlayerbotAI* ai) { return new ShaffarBlinkTrigger(ai); }
    static Trigger* double_breath_danger(PlayerbotAI* ai) { return new YorDoubleBreathTrigger(ai); }
    static Trigger* stomp_danger(PlayerbotAI* ai) { return new YorStompTrigger(ai); }
};

#endif