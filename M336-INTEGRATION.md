# M336 RateButton integration notes

This project incorporates the M336 Rate Button MIT-licensed interface concepts and source portions. The original `LICENSE-M336` notice is retained.

Integrated elements:
- The moderator-rate button in `left-side-menu`, using `GJ_starBtnMod_001.png`.
- M336-style one-time warning explaining the required GDPS `commandRate` permission.
- Star selector, feature/epic/legendary/mythic selector, verified-coins toggle, daily/weekly/send-only actions, and native `!rate`/`!send` comment commands.
- The project adds a separate original-face custom difficulty selector and a `Save Tier` action.

The custom difficulty API in this build uses the Cvolton-compatible PHP add-on in `server-addon/cvolton/database/`. The client posts account ID, GJP2, level ID, and the selected tier over HTTPS. The server verifies GJP2 and `commandRate` permission before writing. Do not point this PHP backend setting at a Supabase project URL.
