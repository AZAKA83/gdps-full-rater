# GDPS Full Rater — integrated source project (v2.1.1)

This source combines M336's Rate Button interface with the GDPS Full Rater custom-tier persistence backend. The selector now uses **original Geometry Dash difficulty sprite frames loaded from the running game**. It does not bundle or copy a third-party difficulty-face collage.

See `M336-INTEGRATION.md` for a summary of the M336 code integration and `scripts/validate_project.py` for source consistency checks.

## Included features

- M336 RateButton moderator button, first-use permission warning, and classic star/feature/coin controls.
- The standard difficulty selector: Auto, Easy, Normal, Hard, Harder, Insane, Easy Demon, Medium Demon, Hard Demon, Insane Demon, Extreme Demon, and Unrated / Remove.
- Original in-game difficulty sprites are referenced by frame name from `GJ_GameSheet03` (for example, `difficulty_01_btn_001.png` through `difficulty_10_btn_001.png`). The game supplies these textures at runtime.
- Fetches a saved custom tier from `gdpsFullRaterGet.php`.
- Saves/removes a custom tier through `gdpsFullRaterSet.php`.
- Reads the logged-in GDPS account ID and GJP2 from `GJAccountManager`; the PHP server validates GJP2 and checks `commandRate` before saving.

## Backend

This build uses the Cvolton/GMDprivateServer-compatible PHP add-on as the active persistence/authentication path. Supabase is not the active rating backend in this build. A Supabase function requires a separate server-side GDPS verification adapter and matching mod requests; a shared token embedded in a public mod is not secure.

Read `server-addon/INSTALL.txt` before installing. Back up the GDPS database and files first. The PHP files in `server-addon/cvolton/database/` belong in the Cvolton server's `database/` directory, and `schema.sql` must be imported into the same MySQL/MariaDB database. This PHP add-on is not automatically compatible with arbitrary Vercel, Supabase, or NitroCore backends; those need a server adapter that implements the same routes and validates the GDPS account and `commandRate` permission.

Expected endpoints:

- `https://YOUR-GDPS-DOMAIN/database/gdpsFullRaterGet.php`
- `https://YOUR-GDPS-DOMAIN/database/gdpsFullRaterSet.php`

Set the mod's **Server URL** setting to that trusted HTTPS folder (keep the trailing slash). **Do not enter your Supabase project URL here**: this build calls the Cvolton PHP endpoints, not Supabase Edge Functions. The user's GJP2 token is sent to this endpoint for server-side validation, so configure only a GDPS host you control or trust.

## Difficulty IDs

The UI and PHP allow-list share these keys:

- `auto`, `easy`, `normal`, `hard`, `harder`, `insane`
- `easy-demon`, `medium-demon`, `hard-demon`, `insane-demon`, `extreme-demon`
- `unrated` removes the custom tier.

The selector is separate from the game's official star rating. The M336 **Stars** button continues to submit the usual GDPS star command; **Save Tier** stores the selected custom difficulty face in the add-on table.

## Build

The metadata targets Geode SDK 5.11.2 and GD 2.2081. C++ compilation has not been completed in this environment because the Geode SDK/build toolchain is not installed here. The source now includes M336's first-use permission warning and improved HTTP response parsing. GD 2.209 compatibility still needs to be tested on-device.

Use `.github/workflows/build.yml` as the single GitHub Actions build workflow. It first validates the difficulty allow-list and PHP syntax, then builds Windows, Android32, and Android64 artifacts. This avoids running both the inherited M336 workflow and the integrated workflow on every push. This archive is source code, not a compiled `.geode` file.

## Attribution and external-image note

The M336 Rate Button source is preserved under its MIT license in `LICENSE-M336`. The requested ArtOvrag page lists the target face image in its image gallery, but the linked image URL was unavailable when checked and the page states that site materials may not be reused without agreement from its administration. For that reason, this package does not copy that third-party image; it uses the original game sprite frames already loaded by Geometry Dash instead.
