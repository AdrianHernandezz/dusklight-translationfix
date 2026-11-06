// Twilight Princess text fixes and restored dialogue.
//
// Replaces the text of a few English messages through Dusklight's MessageService instead of
// overlaying whole message archives. The game keeps loading whichever bmgres*.arc / zel_0N.bmg
// wins (vanilla, Linkle, or another text mod) and only these messages are swapped, so the fixes
// stack with any mod that rewrites other text. Restored cut content (Ganondorf's longer speech
// and the Twili conversations, see twili.cpp) can be switched off in the Mods menu.
#include "settings.h"
#include "twili.h"

#include "mods/service.hpp"
#include "mods/svc/config.h"
#include "mods/svc/hook.h"
#include "mods/svc/log.h"
#include "mods/svc/message.h"
#include "mods/svc/ui.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iterator>

DEFINE_MOD();

IMPORT_SERVICE(LogService, svc_log);
IMPORT_SERVICE(MessageService, svc_message);
IMPORT_SERVICE(HookService, svc_hook);
IMPORT_SERVICE(ConfigService, svc_config);
IMPORT_SERVICE(UiService, svc_ui);

namespace {

struct Fix {
    uint16_t group;  // message group N = bmgresN.arc / zel_0N.bmg
    uint16_t id;
    const uint8_t* text;  // raw BMG text, 0-terminated
    size_t size;
    bool cutContent;  // restored cut dialogue: only while the Ganondorf setting is on
};

#include "fixes.inc"

MessageOverrideHandle g_handles[std::size(kFixes)] = {};
ConfigVarHandle g_cvarGanondorf = 0;
ConfigVarHandle g_cvarTwili = 0;

bool get_bool(ConfigVarHandle handle, bool fallback) {
    bool value = fallback;
    if (handle == 0 || svc_config->get_bool(mod_ctx, handle, &value) != MOD_OK) {
        return fallback;
    }
    return value;
}

// Cut-content lines are resolved each time the game shows them, so the setting applies at once.
bool on_cut_message(ModContext*, const MessageOverrideContext*, MessageTextData* out, void* user) {
    if (!settings::restore_ganondorf()) {
        return false;  // keep the line the game (or another mod) would show
    }
    const Fix& fix = *static_cast<const Fix*>(user);
    out->text = fix.text;
    out->text_size = fix.size;
    return true;
}

ModResult register_bool(ModError* error, const char* name, bool fallback, ConfigVarHandle* out) {
    ConfigVarDesc desc = CONFIG_VAR_DESC_INIT;
    desc.name = name;
    desc.type = CONFIG_VAR_BOOL;
    desc.default_bool = fallback;
    if (svc_config->register_var(mod_ctx, &desc, out) != MOD_OK) {
        return mods::set_error(error, MOD_ERROR, "failed to register a setting");
    }
    return MOD_OK;
}

void add_toggle(UiElementHandle panel, const char* label, const char* help, ConfigVarHandle var) {
    UiControlDesc control = UI_CONTROL_DESC_INIT;
    control.kind = UI_CONTROL_TOGGLE;
    control.label = label;
    control.help_rml = help;
    control.binding = UI_BINDING_CONFIG_VAR;
    control.config_var = var;
    svc_ui->pane_add_control(mod_ctx, panel, &control, nullptr);
}

ModResult build_panel(ModContext*, UiElementHandle panel, void*, ModError*) {
    svc_ui->pane_add_section(mod_ctx, panel, "Restored cut content");
    add_toggle(panel, "Ganondorf's full final speech",
        "Restores the longer speech Ganondorf was meant to give when he is defeated. "
        "Restoration by kelpoo and UltraHylia.",
        g_cvarGanondorf);
    add_toggle(panel, "Twili dialogue",
        "Lets you talk to the Twili in the Twilight Realm once they are freed, using their "
        "original unused lines. Documented by The Cutting Room Floor.",
        g_cvarTwili);
    return MOD_OK;
}

}  // namespace

namespace settings {
bool restore_ganondorf() { return get_bool(g_cvarGanondorf, true); }
bool restore_twili() { return get_bool(g_cvarTwili, true); }
}  // namespace settings

extern "C" {

MOD_EXPORT ModResult mod_initialize(ModError* error) {
    if (register_bool(error, "restoreGanondorfSpeech", true, &g_cvarGanondorf) != MOD_OK ||
        register_bool(error, "restoreTwiliDialogue", true, &g_cvarTwili) != MOD_OK)
    {
        return MOD_ERROR;
    }

    int failed = 0;
    for (size_t i = 0; i < std::size(kFixes); ++i) {
        const Fix& fix = kFixes[i];
        const ModResult result = fix.cutContent ?
            svc_message->override_message_fn(mod_ctx, fix.group, fix.id, MESSAGE_LANGUAGE_ENGLISH,
                on_cut_message, const_cast<Fix*>(&fix), &g_handles[i]) :
            svc_message->override_message(mod_ctx, fix.group, fix.id, MESSAGE_LANGUAGE_ENGLISH,
                fix.text, fix.size, &g_handles[i]);
        if (result != MOD_OK) {
            g_handles[i] = 0;
            ++failed;
        }
    }
    char message[96];
    std::snprintf(message, sizeof(message), "fixed %d of %d messages",
        int(std::size(kFixes)) - failed, int(std::size(kFixes)));
    if (failed > 0) {
        svc_log->warn(mod_ctx, message);
    } else {
        svc_log->info(mod_ctx, message);
    }
    if (!twili_install()) {
        svc_log->warn(mod_ctx, "Twili dialogue could not be restored (hook failed)");
    }

    UiModsPanelDesc panel = UI_MODS_PANEL_DESC_INIT;
    panel.build = build_panel;
    svc_ui->register_mods_panel(mod_ctx, &panel);
    return MOD_OK;
}

MOD_EXPORT ModResult mod_update(ModError* error) {
    return MOD_OK;
}

MOD_EXPORT ModResult mod_shutdown(ModError* error) {
    for (MessageOverrideHandle& handle : g_handles) {
        if (handle != 0) {
            svc_message->remove_override(mod_ctx, handle);
            handle = 0;
        }
    }
    return MOD_OK;
}

}
