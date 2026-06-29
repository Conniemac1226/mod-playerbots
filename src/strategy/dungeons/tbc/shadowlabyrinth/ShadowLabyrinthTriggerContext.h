#ifndef _PLAYERBOT_SHADOWLABYRINTHTRIGGERCONTEXT_H
#define _PLAYERBOT_SHADOWLABYRINTHTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "ShadowLabyrinthTriggers.h"

class ShadowLabyrinthTriggerContext : public NamedObjectContext<Trigger>
{
public:
    ShadowLabyrinthTriggerContext()
    {
        creators["hellmaw corrosive acid"] = &ShadowLabyrinthTriggerContext::hellmaw_corrosive_acid;
        creators["hellmaw fear"] = &ShadowLabyrinthTriggerContext::hellmaw_fear;
        creators["blackheart incite chaos"] = &ShadowLabyrinthTriggerContext::blackheart_incite_chaos;
        creators["blackheart war stomp"] = &ShadowLabyrinthTriggerContext::blackheart_war_stomp;
        creators["blackheart charge"] = &ShadowLabyrinthTriggerContext::blackheart_charge;
        creators["vorpil spread"] = &ShadowLabyrinthTriggerContext::vorpil_spread;
        creators["vorpil rain of fire"] = &ShadowLabyrinthTriggerContext::vorpil_rain_of_fire;
        creators["vorpil draw shadows"] = &ShadowLabyrinthTriggerContext::vorpil_draw_shadows;
        creators["murmur sonic boom"] = &ShadowLabyrinthTriggerContext::murmur_sonic_boom;
        creators["murmur resonance"] = &ShadowLabyrinthTriggerContext::murmur_resonance;
        creators["murmur magnetic pull"] = &ShadowLabyrinthTriggerContext::murmur_magnetic_pull;
        creators["murmur thundering storm"] = &ShadowLabyrinthTriggerContext::murmur_thundering_storm;
    }

private:
    static Trigger* hellmaw_corrosive_acid(PlayerbotAI* botAI) { return new HellmawCorrosiveAcidTrigger(botAI); }
    static Trigger* hellmaw_fear(PlayerbotAI* botAI) { return new HellmawFearTrigger(botAI); }
    static Trigger* blackheart_incite_chaos(PlayerbotAI* botAI) { return new BlackheartInciteChaosTrigger(botAI); }
    static Trigger* blackheart_war_stomp(PlayerbotAI* botAI) { return new BlackheartWarStompTrigger(botAI); }
    static Trigger* blackheart_charge(PlayerbotAI* botAI) { return new BlackheartChargeTrigger(botAI); }
    static Trigger* vorpil_spread(PlayerbotAI* botAI) { return new VorpilSpreadTrigger(botAI); }
    static Trigger* vorpil_rain_of_fire(PlayerbotAI* botAI) { return new VorpilRainOfFireTrigger(botAI); }
    static Trigger* vorpil_draw_shadows(PlayerbotAI* botAI) { return new VorpilDrawShadowsTrigger(botAI); }
    static Trigger* murmur_sonic_boom(PlayerbotAI* botAI) { return new MurmurSonicBoomTrigger(botAI); }
    static Trigger* murmur_resonance(PlayerbotAI* botAI) { return new MurmurResonanceTrigger(botAI); }
    static Trigger* murmur_magnetic_pull(PlayerbotAI* botAI) { return new MurmurMagneticPullTrigger(botAI); }
    static Trigger* murmur_thundering_storm(PlayerbotAI* botAI) { return new MurmurThunderingStormTrigger(botAI); }
};

#endif
