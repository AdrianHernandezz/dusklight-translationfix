# Fixed Mistranslations & Restored Dialogue

Native Dusklight mod (`com.example.translationfix`, version 2.3.5). It fixes a few English
mistranslations in Twilight Princess and restores two pieces of cut dialogue.

## How it works

The mod does not overlay any message archive. At startup it replaces individual English
messages through Dusklight's MessageService (`override_message`), keyed by message group and
message ID. Because only those messages are touched, it works on top of whatever message files
are loaded: vanilla, the Linkle Mod, or another text mod. One .dusk covers every case.

| Group (bmgresN) | Message | Change |
|---|---|---|
| 2 | 6217 | Goron rockslide line: "has blocked the way ... for some time now" (continuity fix) |
| 4 | 5609 | Auru: "I was appointed to be Princess Zelda's tutor" |
| 4 | 5610 | Auru: "them" -> "then" |
| 4 | 5716, 5717 | Shad: a race closer to the gods created Hyrule / the Hylians' new capital |
| 4 | 5718 | Shad: space after "..." ("They dwelt there... and some") |
| 0 | 3726, 3727 | Restored Ganondorf dialogue by kelpoo and UltraHylia (toggleable) |

### Twili dialogue (toggleable)

The Twili in the Palace of Twilight (`daNpc_yamiD_c`, `yamiS_c`, `yamiT_c`) still have their
conversations in the game data (flows 801-806 in message group 5, messages 9701-9726), but
`setParam()` always sets their attention flags to 0, so Link can never talk to them.
`src/twili.cpp` post-hooks `setAttnPos()` on all three classes and turns the talk prompt back on
when the Twili has a flow and is visible. The text is the game's own unused text.

### Settings

Both restorations can be switched off in the Mods menu ("Restored cut content"):
`restoreGanondorfSpeech` and `restoreTwiliDialogue`, both on by default. The Ganondorf lines use
`override_message_fn`, so the setting is checked live each time the message is shown.

## Source layout

- `src/mod.cpp`: mod entry point, settings, Mods-menu panel, message overrides.
- `src/twili.cpp`: the Twili talk hooks.
- `src/fixes.inc`: generated message texts, do not edit by hand.
- `tools/gen_fixes.py`: regenerates `src/fixes.inc`:
  `python3 -I tools/gen_fixes.py <vanilla bmgres2.arc> <vanilla bmgres4.arc> translationfix_1.0.0_original.zip tools/ganondorf_restoration_bmgres.arc > src/fixes.inc`
- `tools/merge_mod.py`: packs the build into a .dusk and embeds the imported symbols.

## Building

Build with the Dusklight SDK (v2.0.2) once per target platform, then merge the per-platform
bundles into one .dusk:

    python3 tools/merge_mod.py -o com.example.translationfix_2.3.5.dusk --symgen symgen \
        build-windows/mods/translationfix.dusk build-linux-x86_64/mods/translationfix.dusk ...

The 2.3.5 release contains builds for Windows x64, Linux x86_64, Linux arm64, macOS Apple
Silicon (arm64), macOS Intel (x86_64) and Android (arm64). Windows and macOS are
cross-compiled with clang 18; `tools/toolchains/macos.cmake` is the macOS toolchain file (pass
`-DMAC_ARCH=arm64` or `x86_64` and point `CMAKE_OSX_SYSROOT` at a macOS SDK).

Android uses the NDK's own toolchain file (NDK r29, API level 28):

    cmake -B build-android -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
        -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-28
    cmake --build build-android

The SDK downloads the Android link stub (`stub-android-aarch64.so`) by itself.

## Credits

- Auru's retranslation by ahrbro.
- Shad's retranslation comes mostly from Anna Cairistiona's translation, with a small rewording by ahrbro.
- The Goron rockslide continuity error was pointed out by Zelda Wiki.
- Restored Ganondorf dialogue by kelpoo and UltraHylia.
- The Twili dialogue is the game's own unused text, documented by The Cutting Room Floor.
- The mod icon was inspired by EnforcerRidley.
- Claude (Anthropic) wrote only the code: the Linkle Mod compatibility and the Twili talk fix. It wrote none of the dialogue.
