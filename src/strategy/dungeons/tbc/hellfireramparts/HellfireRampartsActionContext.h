#ifndef _PLAYERBOT_HELLFIRERAMPARTSACTIONCONTEXT_H
#define _PLAYERBOT_HELLFIRERAMPARTSACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "HellfireRampartsActions.h"

class HellfireRampartsActionContext : public NamedObjectContext<Action>
{
public:
    HellfireRampartsActionContext()
    {
        creators["attack hellfire watcher"] = &HellfireRampartsActionContext::attack_hellfire_watcher;
        creators["gargolmar retaliation"] = &HellfireRampartsActionContext::gargolmar_retaliation;
        creators["gargolmar surge"] = &HellfireRampartsActionContext::gargolmar_surge;
        creators["attack fiendish hound"] = &HellfireRampartsActionContext::attack_fiendish_hound;
        creators["interrupt omor shadow bolt"] = &HellfireRampartsActionContext::interrupt_omor_shadow_bolt;
        creators["omor treacherous aura"] = &HellfireRampartsActionContext::omor_treacherous_aura;
        creators["omor demonic shield"] = &HellfireRampartsActionContext::omor_demonic_shield;
        creators["omor proactive spread"] = &HellfireRampartsActionContext::omor_proactive_spread;
        creators["avoid liquid fire"] = &HellfireRampartsActionContext::avoid_liquid_fire;
        creators["avoid cone of fire"] = &HellfireRampartsActionContext::avoid_cone_of_fire;
        creators["attack nazan first"] = &HellfireRampartsActionContext::attack_nazan_first;
        creators["attack vazruden"] = &HellfireRampartsActionContext::attack_vazruden;
        creators["nazan bellowing roar"] = &HellfireRampartsActionContext::nazan_bellowing_roar;
    }

private:
    static Action* attack_hellfire_watcher(PlayerbotAI* ai) { return new AttackHellfireWatcherAction(ai); }
    static Action* gargolmar_retaliation(PlayerbotAI* ai) { return new GargolmarRetaliationAction(ai); }
    static Action* gargolmar_surge(PlayerbotAI* ai) { return new GargolmarSurgeAction(ai); }
    static Action* attack_fiendish_hound(PlayerbotAI* ai) { return new AttackFiendishHoundAction(ai); }
    static Action* interrupt_omor_shadow_bolt(PlayerbotAI* ai) { return new OmorShadowBoltInterruptAction(ai); }
    static Action* omor_treacherous_aura(PlayerbotAI* ai) { return new OmorTreacherousAuraAction(ai); }
    static Action* omor_demonic_shield(PlayerbotAI* ai) { return new OmorDemonicShieldAction(ai); }
    static Action* omor_proactive_spread(PlayerbotAI* ai) { return new OmorProactiveSpreadAction(ai); }
    static Action* avoid_liquid_fire(PlayerbotAI* ai) { return new NazanLiquidFireAction(ai); }
    static Action* avoid_cone_of_fire(PlayerbotAI* ai) { return new NazanConeOfFireAction(ai); }
    static Action* attack_nazan_first(PlayerbotAI* ai) { return new AttackNazanFirstAction(ai); }
    static Action* attack_vazruden(PlayerbotAI* ai) { return new AttackVazrudenAction(ai); }
    static Action* nazan_bellowing_roar(PlayerbotAI* ai) { return new NazanBellowingRoarAction(ai); }
};

#endif