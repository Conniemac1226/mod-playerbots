#ifndef _PLAYERBOT_SLAVEPENSTRIGGERCONTEXT_H
#define _PLAYERBOT_SLAVEPENSTRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "SlavePensTriggers.h"

class SlavePensTriggerContext : public NamedObjectContext<Trigger>
{
public:
    SlavePensTriggerContext()
    {
        creators["mennu totem active"] = &SlavePensTriggerContext::mennu_totem_active;
        creators["mennu lightning bolt cast"] = &SlavePensTriggerContext::mennu_lightning_bolt_cast;
        creators["nova totem nearby"] = &SlavePensTriggerContext::nova_totem_nearby;
        creators["ensnaring moss active"] = &SlavePensTriggerContext::ensnaring_moss_active;
        creators["grievous wound active"] = &SlavePensTriggerContext::grievous_wound_active;
        creators["water spit incoming"] = &SlavePensTriggerContext::water_spit_incoming;
        creators["acid spray casting"] = &SlavePensTriggerContext::acid_spray_casting;
        creators["poison bolt volley casting"] = &SlavePensTriggerContext::poison_bolt_volley_casting;
        creators["uppercut positioning"] = &SlavePensTriggerContext::uppercut_positioning;
        creators["quagmirran cleave positioning"] = &SlavePensTriggerContext::quagmirran_cleave_positioning;
    }

private:
    static Trigger* mennu_totem_active(PlayerbotAI* ai) { return new MennuTotemActiveTrigger(ai); }
    static Trigger* mennu_lightning_bolt_cast(PlayerbotAI* ai) { return new MennuLightningBoltCastTrigger(ai); }
    static Trigger* nova_totem_nearby(PlayerbotAI* ai) { return new MennuNovaTotemNearbyTrigger(ai); }
    static Trigger* ensnaring_moss_active(PlayerbotAI* ai) { return new RokmarEnsnaringMossTrigger(ai); }
    static Trigger* grievous_wound_active(PlayerbotAI* ai) { return new RokmarGrievousWoundTrigger(ai); }
    static Trigger* water_spit_incoming(PlayerbotAI* ai) { return new RokmarWaterSpitTrigger(ai); }
    static Trigger* acid_spray_casting(PlayerbotAI* ai) { return new QuagmirranAcidSprayTrigger(ai); }
    static Trigger* poison_bolt_volley_casting(PlayerbotAI* ai) { return new QuagmirranPoisonBoltVolleyTrigger(ai); }
    static Trigger* uppercut_positioning(PlayerbotAI* ai) { return new QuagmirranUppercutTrigger(ai); }
    static Trigger* quagmirran_cleave_positioning(PlayerbotAI* ai) { return new QuagmirranCleavePositionTrigger(ai); }
};

#endif