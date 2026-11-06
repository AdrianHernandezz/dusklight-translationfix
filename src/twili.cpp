// Restores the Twili's dialogue in the Twilight Realm (Palace of Twilight).
//
// The three Twili NPC actors (daNpc_yamiD_c / yamiS_c / yamiT_c) still carry a complete
// conversation setup: each one reads its flow number from the map (or flow 806 once the Palace of
// Twilight is cleared), and evtTalk()/talk() still run initTalk() with it. The flows (801-806 in
// message group 5, messages 9701-9726) are intact too. The only thing switched off is the talk
// prompt: setParam() always sets attention_info.flags to 0, so Link can never start a
// conversation. After setAttnPos() (the last step of the actor's execute before collision) this
// re-enables the talk/speak attention whenever the Twili has a conversation and is visible.
#include "twili.h"
#include "settings.h"

#include "mods/svc/hook.hpp"

#include "d/actor/d_a_npc_yamid.h"
#include "d/actor/d_a_npc_yamis.h"
#include "d/actor/d_a_npc_yamit.h"

DEFINE_HOOK(&daNpc_yamiD_c::setAttnPos, YamiDSetAttnPos);
DEFINE_HOOK(&daNpc_yamiS_c::setAttnPos, YamiSSetAttnPos);
DEFINE_HOOK(&daNpc_yamiT_c::setAttnPos, YamiTSetAttnPos);

namespace {

template <class T>
void allow_talk(T* npc) {
    if (npc == nullptr || npc->mFlowNodeNo < 0 || !settings::restore_twili()) {
        return;  // no conversation placed for this Twili
    }
    if (npc->checkHide() || npc->mNoDraw || npc->is_vanish()) {
        return;
    }
    npc->attention_info.flags |= fopAc_AttnFlag_SPEAK_e | fopAc_AttnFlag_TALK_e;
}

template <class T>
void on_set_attn_pos_post(ModContext*, void* args, void*, void*) {
    allow_talk(mods::arg<T*>(args, 0));
}

}  // namespace

bool twili_install() {
    return mods::hook::add_post<YamiDSetAttnPos>(on_set_attn_pos_post<daNpc_yamiD_c>) == MOD_OK &&
           mods::hook::add_post<YamiSSetAttnPos>(on_set_attn_pos_post<daNpc_yamiS_c>) == MOD_OK &&
           mods::hook::add_post<YamiTSetAttnPos>(on_set_attn_pos_post<daNpc_yamiT_c>) == MOD_OK;
}
