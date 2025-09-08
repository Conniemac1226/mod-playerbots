#ifndef _PLAYERBOT_GRUULTRIGGERCONTEXT_H
#define _PLAYERBOT_GRUULTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "GruulTriggers.h"

class GruulTriggerContext : public NamedObjectContext<Trigger>
{
public:
    GruulTriggerContext()
    {
        creators["gruul engaged"] = &GruulTriggerContext::gruul_engaged;
        creators["gruul ground slam"] = &GruulTriggerContext::gruul_ground_slam;
        creators["gruul shatter"] = &GruulTriggerContext::gruul_shatter;
        creators["gruul cave in"] = &GruulTriggerContext::gruul_cave_in;
        creators["gruul reverberation"] = &GruulTriggerContext::gruul_reverberation;
        creators["gruul hurtful strike"] = &GruulTriggerContext::gruul_hurtful_strike;
        creators["gruul growth"] = &GruulTriggerContext::gruul_growth;
        
        creators["maulgar engaged"] = &GruulTriggerContext::maulgar_engaged;
        creators["maulgar council"] = &GruulTriggerContext::maulgar_council;
        creators["maulgar whirlwind"] = &GruulTriggerContext::maulgar_whirlwind;
        creators["maulgar arcing smash"] = &GruulTriggerContext::maulgar_arcing_smash;
        creators["maulgar berserker"] = &GruulTriggerContext::maulgar_berserker;
        
        creators["krosh spellshield"] = &GruulTriggerContext::krosh_spellshield;
        creators["krosh blast wave"] = &GruulTriggerContext::krosh_blast_wave;
        
        creators["kiggler polymorph"] = &GruulTriggerContext::kiggler_polymorph;
        creators["kiggler arcane explosion"] = &GruulTriggerContext::kiggler_arcane_explosion;
        
        creators["olm wild fel stalker"] = &GruulTriggerContext::olm_wild_fel_stalker;
        
        creators["blindeye heal"] = &GruulTriggerContext::blindeye_heal;
        creators["blindeye shield"] = &GruulTriggerContext::blindeye_shield;
    }
    
private:
    // Gruul triggers
    static Trigger* gruul_engaged(PlayerbotAI* ai) { return new GruulEngagedTrigger(ai); }
    static Trigger* gruul_ground_slam(PlayerbotAI* ai) { return new GruulGroundSlamTrigger(ai); }
    static Trigger* gruul_shatter(PlayerbotAI* ai) { return new GruulShatterTrigger(ai); }
    static Trigger* gruul_cave_in(PlayerbotAI* ai) { return new GruulCaveInTrigger(ai); }
    static Trigger* gruul_reverberation(PlayerbotAI* ai) { return new GruulReverberationTrigger(ai); }
    static Trigger* gruul_hurtful_strike(PlayerbotAI* ai) { return new GruulHurtfulStrikeTrigger(ai); }
    static Trigger* gruul_growth(PlayerbotAI* ai) { return new GruulGrowthTrigger(ai); }
    
    // Maulgar triggers
    static Trigger* maulgar_engaged(PlayerbotAI* ai) { return new MaulgarEngagedTrigger(ai); }
    static Trigger* maulgar_council(PlayerbotAI* ai) { return new MaulgarCouncilTrigger(ai); }
    static Trigger* maulgar_whirlwind(PlayerbotAI* ai) { return new MaulgarWhirlwindTrigger(ai); }
    static Trigger* maulgar_arcing_smash(PlayerbotAI* ai) { return new MaulgarArcingSmashTrigger(ai); }
    static Trigger* maulgar_berserker(PlayerbotAI* ai) { return new MaulgarBerserkerTrigger(ai); }
    
    // Council triggers
    static Trigger* krosh_spellshield(PlayerbotAI* ai) { return new KroshSpellshieldTrigger(ai); }
    static Trigger* krosh_blast_wave(PlayerbotAI* ai) { return new KroshBlastWaveTrigger(ai); }
    static Trigger* kiggler_polymorph(PlayerbotAI* ai) { return new KigglerPolymorphTrigger(ai); }
    static Trigger* kiggler_arcane_explosion(PlayerbotAI* ai) { return new KigglerArcaneExplosionTrigger(ai); }
    static Trigger* olm_wild_fel_stalker(PlayerbotAI* ai) { return new OlmWildFelStalkerTrigger(ai); }
    static Trigger* blindeye_heal(PlayerbotAI* ai) { return new BlindeyeHealTrigger(ai); }
    static Trigger* blindeye_shield(PlayerbotAI* ai) { return new BlindeyeShieldTrigger(ai); }
};

#endif