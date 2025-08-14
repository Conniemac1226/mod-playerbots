#pragma once
#include "NamedObjectContext.h"
#include "BlackMorassTriggers.h"

class BlackMorassTriggerContext : public NamedObjectContext<Trigger>
{
public:
    BlackMorassTriggerContext()
    {
        // Portal/Add Management (using AI_VALUE triggers)
        creators["portal add active"] = &BlackMorassTriggerContext::portal_add_active;
        creators["medivh needs protection"] = &BlackMorassTriggerContext::medivh_needs_protection;
        
        // Aeonus
        creators["aeonus cleave danger"] = &BlackMorassTriggerContext::aeonus_cleave_nearby;
        creators["aeonus engaged"] = &BlackMorassTriggerContext::aeonus_engaged;
        creators["sand breath danger"] = &BlackMorassTriggerContext::sand_breath_danger;
        creators["time stop active"] = &BlackMorassTriggerContext::time_stop_active;
        creators["aeonus enraged"] = &BlackMorassTriggerContext::aeonus_enraged;
        
        // Chrono Lord Deja
        creators["chrono lord deja engaged"] = &BlackMorassTriggerContext::chrono_lord_deja_engaged;
        creators["time lapse danger"] = &BlackMorassTriggerContext::time_lapse_nearby;
        creators["arcane discharge danger"] = &BlackMorassTriggerContext::arcane_discharge_nearby;
        creators["attraction active"] = &BlackMorassTriggerContext::attraction_active;
        creators["deja arcane blast casting"] = &BlackMorassTriggerContext::deja_arcane_blast_casting;
        
        // Temporus
        creators["temporus engaged"] = &BlackMorassTriggerContext::temporus_engaged;
        creators["wing buffet danger"] = &BlackMorassTriggerContext::wing_buffet_nearby;
        creators["mortal wound active"] = &BlackMorassTriggerContext::mortal_wound_active;
        creators["temporus reflect active"] = &BlackMorassTriggerContext::temporus_reflect_active;
        creators["temporus hasten active"] = &BlackMorassTriggerContext::temporus_hasten_active;
    }

private:
    // Portal/Add Management (safe trigger using Value<>* pattern to prevent crashes)
    static Trigger* portal_add_active(PlayerbotAI* botAI) { 
        class SafeTrigger : public Trigger {
        public:
            SafeTrigger(PlayerbotAI* ai) : Trigger(ai, "portal add active") {}
            bool IsActive() override { 
                // FIXED: Use safe Value<>* pattern to prevent crashes (matches action pattern)
                Value<bool>* boolValue = botAI->GetAiObjectContext()->GetValue<bool>("portal add active");
                return boolValue ? boolValue->Get() : false;
            }
        };
        return new SafeTrigger(botAI);
    }
    
    static Trigger* medivh_needs_protection(PlayerbotAI* botAI) {
        class SafeTrigger : public Trigger {
        public:
            SafeTrigger(PlayerbotAI* ai) : Trigger(ai, "medivh needs protection") {}
            bool IsActive() override { 
                Value<bool>* boolValue = botAI->GetAiObjectContext()->GetValue<bool>("medivh needs protection");
                return boolValue ? boolValue->Get() : false;
            }
        };
        return new SafeTrigger(botAI);
    }
    
    static Trigger* sand_breath_danger(PlayerbotAI* botAI) {
        class SafeTrigger : public Trigger {
        public:
            SafeTrigger(PlayerbotAI* ai) : Trigger(ai, "sand breath danger") {}
            bool IsActive() override { 
                Value<bool>* boolValue = botAI->GetAiObjectContext()->GetValue<bool>("sand breath danger");
                return boolValue ? boolValue->Get() : false;
            }
        };
        return new SafeTrigger(botAI);
    }
    
    static Trigger* time_stop_active(PlayerbotAI* botAI) {
        class SafeTrigger : public Trigger {
        public:
            SafeTrigger(PlayerbotAI* ai) : Trigger(ai, "time stop active") {}
            bool IsActive() override { 
                Value<bool>* boolValue = botAI->GetAiObjectContext()->GetValue<bool>("time stop active");
                return boolValue ? boolValue->Get() : false;
            }
        };
        return new SafeTrigger(botAI);
    }
    
    static Trigger* attraction_active(PlayerbotAI* botAI) {
        class SafeTrigger : public Trigger {
        public:
            SafeTrigger(PlayerbotAI* ai) : Trigger(ai, "attraction active") {}
            bool IsActive() override { 
                Value<bool>* boolValue = botAI->GetAiObjectContext()->GetValue<bool>("attraction active");
                return boolValue ? boolValue->Get() : false;
            }
        };
        return new SafeTrigger(botAI);
    }
    
    static Trigger* mortal_wound_active(PlayerbotAI* botAI) {
        class SafeTrigger : public Trigger {
        public:
            SafeTrigger(PlayerbotAI* ai) : Trigger(ai, "mortal wound active") {}
            bool IsActive() override { 
                Value<bool>* boolValue = botAI->GetAiObjectContext()->GetValue<bool>("mortal wound active");
                return boolValue ? boolValue->Get() : false;
            }
        };
        return new SafeTrigger(botAI);
    }
    
    static Trigger* temporus_reflect_active(PlayerbotAI* botAI) {
        class SafeTrigger : public Trigger {
        public:
            SafeTrigger(PlayerbotAI* ai) : Trigger(ai, "temporus reflect active") {}
            bool IsActive() override { 
                Value<bool>* boolValue = botAI->GetAiObjectContext()->GetValue<bool>("temporus reflect active");
                return boolValue ? boolValue->Get() : false;
            }
        };
        return new SafeTrigger(botAI);
    }
    
    // Aeonus
    static Trigger* aeonus_cleave_nearby(PlayerbotAI* botAI) { return new AeonusCleaveNearbyTrigger(botAI); }
    static Trigger* aeonus_engaged(PlayerbotAI* botAI) { return new AeonusEngagedTrigger(botAI); }
    static Trigger* aeonus_enraged(PlayerbotAI* botAI) { return new AeonusEnragedTrigger(botAI); }
    
    // Chrono Lord Deja
    static Trigger* chrono_lord_deja_engaged(PlayerbotAI* botAI) { return new ChronoLordDejaEngagedTrigger(botAI); }
    static Trigger* time_lapse_nearby(PlayerbotAI* botAI) { return new TimeLapseNearbyTrigger(botAI); }
    static Trigger* arcane_discharge_nearby(PlayerbotAI* botAI) { return new ArcaneDischargeNearbyTrigger(botAI); }
    static Trigger* deja_arcane_blast_casting(PlayerbotAI* botAI) { return new DejaArcaneBlastCastingTrigger(botAI); }
    
    // Temporus
    static Trigger* temporus_engaged(PlayerbotAI* botAI) { return new TemporusEngagedTrigger(botAI); }
    static Trigger* wing_buffet_nearby(PlayerbotAI* botAI) { return new WingBuffetNearbyTrigger(botAI); }
    static Trigger* temporus_hasten_active(PlayerbotAI* botAI) { return new TemporusHastenActiveTrigger(botAI); }
};