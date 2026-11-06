# Fixed Major Mistranslations & Mistakes

Native Dusklight mod (`com.example.translationfix`). Since 2.0.0 it no longer overlays
`res/Msgus/bmgres2.arc` / `bmgres4.arc`. It replaces 8 English messages at runtime through
Dusklight's MessageService, so it works on top of whatever message files are loaded (vanilla,
the Linkle Mod, or another text mod) and one .dusk covers every case.

| Group (bmgresN) | Message | Fix |
|---|---|---|
| 2 | 6217 | rockslide "has blocked the way ... for some time now" |
| 4 | 5609 | "I was appointed to be Princess Zelda's tutor" |
| 4 | 5610 | "them" -> "then" |
| 4 | 5716, 5717 | race closer to the gods created Hyrule / the Hylians' new capital |
| 4 | 5718 | space after "..." ("They dwelt there... and some") |
| 0 | 3726, 3727 | restored cut Ganondorf dialogue, by kelpoo and UltraHylia (tools/ganondorf_restoration_bmgres.arc) |

- `src/fixes.inc` is generated: `python3 -I tools/gen_fixes.py <vanilla bmgres2.arc> <vanilla bmgres4.arc> translationfix_1.0.0_original.zip tools/ganondorf_restoration_bmgres.arc > src/fixes.inc`
- Build (Windows): see the project's cross-build recipe; then
  `python3 tools/merge_mod.py -o com.example.translationfix_2.0.0.dusk --symgen symgen build/mods/translationfix.dusk`
- `old-1.1.0-overlay/` holds the earlier overlay builds (vanilla and Linkle editions), kept for reference.

## Twili dialogue (2.2.0)

The Twili NPCs in the Palace of Twilight (`daNpc_yamiD_c`, `yamiS_c`, `yamiT_c`) still have their
conversations (flows 801-806 in message group 5, messages 9701-9726), but `setParam()` always sets
their attention flags to 0 so Link can't talk to them. `src/twili.cpp` post-hooks `setAttnPos()` on
all three and turns the talk prompt back on when the Twili has a flow and is visible. Special
thanks to The Cutting Room Floor for documenting the unused text.
