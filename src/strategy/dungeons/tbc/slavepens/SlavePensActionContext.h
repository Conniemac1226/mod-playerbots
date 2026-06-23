#ifndef _PLAYERBOT_SLAVEPENSACTIONCONTEXT_H
#define _PLAYERBOT_SLAVEPENSACTIONCONTEXT_H

#include "AiObjectContext.h"
#include "SlavePensActions.h"

class SlavePensActionContext : public NamedObjectContext<Action>
{
public:
    SlavePensActionContext()
    {
        creators["attack mennu totem"] = &SlavePensActionContext::attack_mennu_totem;
        creators["interrupt mennu lightning bolt"] = &SlavePensActionContext::interrupt_mennu_lightning_bolt;
        creators["avoid nova totem"] = &SlavePensActionContext::avoid_nova_totem;
        creators["dispel ensnaring moss"] = &SlavePensActionContext::dispel_ensnaring_moss;
        creators["rokmar frenzy"] = &SlavePensActionContext::rokmar_frenzy;
        creators["heal grievous wound"] = &SlavePensActionContext::heal_grievous_wound;
        creators["avoid water spit"] = &SlavePensActionContext::avoid_water_spit;
        creators["avoid acid spray"] = &SlavePensActionContext::avoid_acid_spray;
        creators["interrupt poison bolt volley"] = &SlavePensActionContext::interrupt_poison_bolt_volley;
        creators["tank position uppercut"] = &SlavePensActionContext::tank_position_uppercut;
        creators["avoid quagmirran cleave"] = &SlavePensActionContext::avoid_quagmirran_cleave;
    }

private:
    static Action* attack_mennu_totem(PlayerbotAI* ai) { return new AttackMennuTotemAction(ai); }
    static Action* interrupt_mennu_lightning_bolt(PlayerbotAI* ai) { return new MennuLightningBoltInterruptAction(ai); }
    static Action* avoid_nova_totem(PlayerbotAI* ai) { return new MennuNovaTotemAction(ai); }
    static Action* dispel_ensnaring_moss(PlayerbotAI* ai) { return new RokmarEnsnaringMossAction(ai); }
    static Action* rokmar_frenzy(PlayerbotAI* ai) { return new RokmarFrenzyAction(ai); }
    static Action* heal_grievous_wound(PlayerbotAI* ai) { return new RokmarGrievousWoundAction(ai); }
    static Action* avoid_water_spit(PlayerbotAI* ai) { return new RokmarWaterSpitAction(ai); }
    static Action* avoid_acid_spray(PlayerbotAI* ai) { return new QuagmirranAcidSprayAction(ai); }
    static Action* interrupt_poison_bolt_volley(PlayerbotAI* ai) { return new QuagmirranPoisonBoltVolleyAction(ai); }
    static Action* tank_position_uppercut(PlayerbotAI* ai) { return new QuagmirranUppercutAction(ai); }
    static Action* avoid_quagmirran_cleave(PlayerbotAI* ai) { return new QuagmirranCleavePositionAction(ai); }
};

#endif
