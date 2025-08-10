#ifndef _PLAYERBOT_SHATTEREDHALLSTRIGGERCONTEXT_H
#define _PLAYERBOT_SHATTEREDHALLSTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "ShatteredHallsTriggers.h"

class ShatteredHallsTriggerContext : public NamedObjectContext<Trigger>
{
public:
    ShatteredHallsTriggerContext()
    {
        creators["nethekurse shadow fissure"] = &ShatteredHallsTriggerContext::nethekurse_shadow_fissure;
        creators["nethekurse dark spin"] = &ShatteredHallsTriggerContext::nethekurse_dark_spin;
        creators["nethekurse peons"] = &ShatteredHallsTriggerContext::nethekurse_peons;
        creators["omrogg blast wave"] = &ShatteredHallsTriggerContext::omrogg_blast_wave;
        creators["omrogg burning maul"] = &ShatteredHallsTriggerContext::omrogg_burning_maul;
        creators["kargath blade dance"] = &ShatteredHallsTriggerContext::kargath_blade_dance;
        creators["kargath assassins"] = &ShatteredHallsTriggerContext::kargath_assassins;
    }

private:
    static Trigger* nethekurse_shadow_fissure(PlayerbotAI* botAI) { return new NethekurseShadowFissureTrigger(botAI); }
    static Trigger* nethekurse_dark_spin(PlayerbotAI* botAI) { return new NethekurseDarkSpinTrigger(botAI); }
    static Trigger* nethekurse_peons(PlayerbotAI* botAI) { return new NethekursePeonsTrigger(botAI); }
    static Trigger* omrogg_blast_wave(PlayerbotAI* botAI) { return new OmroggBlastWaveTrigger(botAI); }
    static Trigger* omrogg_burning_maul(PlayerbotAI* botAI) { return new OmroggBurningMaulTrigger(botAI); }
    static Trigger* kargath_blade_dance(PlayerbotAI* botAI) { return new KargathBladeDanceTrigger(botAI); }
    static Trigger* kargath_assassins(PlayerbotAI* botAI) { return new KargathAssassinsTrigger(botAI); }
};

#endif