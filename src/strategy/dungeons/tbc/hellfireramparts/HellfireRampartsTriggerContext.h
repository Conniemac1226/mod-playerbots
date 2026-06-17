#ifndef _PLAYERBOT_HELLFIRERAMPARTSTRIGGERCONTEXT_H
#define _PLAYERBOT_HELLFIRERAMPARTSTRIGGERCONTEXT_H

#include "Trigger.h"
#include "NamedObjectContext.h"
#include "HellfireRampartsTriggers.h"

class HellfireRampartsTriggerContext : public NamedObjectContext<Trigger>
{
public:
    HellfireRampartsTriggerContext()
    {
        creators["hellfire watcher active"] = &HellfireRampartsTriggerContext::hellfire_watcher_active;
        creators["gargolmar retaliation"] = &HellfireRampartsTriggerContext::gargolmar_retaliation;
        creators["fiendish hound active"] = &HellfireRampartsTriggerContext::fiendish_hound_active;
        creators["omor shadow bolt cast"] = &HellfireRampartsTriggerContext::omor_shadow_bolt_cast;
        creators["omor treacherous aura"] = &HellfireRampartsTriggerContext::omor_treacherous_aura;
        creators["omor demonic shield"] = &HellfireRampartsTriggerContext::omor_demonic_shield;
        creators["liquid fire nearby"] = &HellfireRampartsTriggerContext::liquid_fire_nearby;
        creators["nazan cone of fire"] = &HellfireRampartsTriggerContext::nazan_cone_of_fire;
        creators["nazan landed"] = &HellfireRampartsTriggerContext::nazan_landed;
        creators["vazruden alone"] = &HellfireRampartsTriggerContext::vazruden_alone;
        creators["nazan bellowing roar"] = &HellfireRampartsTriggerContext::nazan_bellowing_roar;
    }

private:
    static Trigger* hellfire_watcher_active(PlayerbotAI* ai) { return new HellfireWatcherActiveTrigger(ai); }
    static Trigger* gargolmar_retaliation(PlayerbotAI* ai) { return new GargolmarRetaliationTrigger(ai); }
    static Trigger* fiendish_hound_active(PlayerbotAI* ai) { return new FiendishHoundActiveTrigger(ai); }
    static Trigger* omor_shadow_bolt_cast(PlayerbotAI* ai) { return new OmorShadowBoltCastTrigger(ai); }
    static Trigger* omor_treacherous_aura(PlayerbotAI* ai) { return new OmorTreacherousAuraTrigger(ai); }
    static Trigger* omor_demonic_shield(PlayerbotAI* ai) { return new OmorDemonicShieldTrigger(ai); }
    static Trigger* liquid_fire_nearby(PlayerbotAI* ai) { return new LiquidFireNearbyTrigger(ai); }
    static Trigger* nazan_cone_of_fire(PlayerbotAI* ai) { return new NazanConeOfFireTrigger(ai); }
    static Trigger* nazan_landed(PlayerbotAI* ai) { return new NazanLandedTrigger(ai); }
    static Trigger* vazruden_alone(PlayerbotAI* ai) { return new VazrudenAloneTrigger(ai); }
    static Trigger* nazan_bellowing_roar(PlayerbotAI* ai) { return new NazanBellowingRoarTrigger(ai); }
};

#endif
