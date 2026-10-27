# GDPS Full Rater (real server-backed custom ratings)

This Geode mod lets an authorized moderator store one custom difficulty tier per online level on a compatible Cvolton/GMDprivateServer GDPS. It uses HTTPS calls to the companion PHP endpoints and the currently logged-in game's account token in memory; it does not save credentials into a file.

## Important: server installation is required

The mod cannot save ratings on GitHub by itself. Before rating works, install the server add-on on the **same Cvolton GDPS** your Geometry Dash client connects to:

1. Back up the GDPS files and database.
2. Import `server-addon/cvolton/schema.sql` into the same MySQL/MariaDB database used by Cvolton.
3. Copy `server-addon/cvolton/gdpsFullRaterSet.php` and `server-addon/cvolton/gdpsFullRaterGet.php` to the same Cvolton API directory as `rateGJStars20.php` (commonly the `database/` directory), where their relative `incl/lib/` paths resolve.
4. Keep HTTPS enabled. Do not put GJP2/password tokens or database credentials in this public source repository.
5. Confirm the endpoints are reachable, for example `https://YOUR-GDPS-URL/database/gdpsFullRaterGet.php?levelID=1`. A valid no-rating response looks like `{"ok":true,"levelID":1,"rating":null}`.
6. In Geometry Dash, open **Geode → Mods → GDPS Full Rater** and set **GDPS Server URL** to the HTTPS URL of the API directory that contains the two PHP files (commonly ending in `/database`), without a trailing slash.
7. The account must have Cvolton's `commandRate` permission. The write endpoint checks it on every submission.

## In-game use

1. Sign in to your GDPS account inside the GDPS client.
2. Open an uploaded/online level's info page and tap the compact **RATE** button.
3. The mod reads the current custom rating from the server.
4. Tap **Next tier** until the desired extra tier is displayed, then tap **Save rating**. Selecting **Remove custom rating** deletes the stored custom tier.
5. The mod displays the server's response. `MISSING_COMMAND_RATE_PERMISSION` means the server owner must grant `commandRate`; `LEVEL_NOT_FOUND` means the level ID is not in that GDPS database.

## What “real rating” means here

Custom tiers are saved in `gdps_full_rater_custom_difficulties` and are shared across clients using this mod on this GDPS. This is a real server-side record, not a local-only button. However, it is **a custom tier layer**: it does not automatically alter Cvolton's native star count, featured status, demon flag, or the built-in difficulty shown in unmodified Geometry Dash. Those native ratings still need the GDPS's normal rating workflow. Only compatible clients using this mod can display these extra custom tiers unless you also modify other server/client pages.

## Build

The custom tier is a persistent server-side record that compatible clients can read. It does not alter Cvolton’s normal built-in stars/featured/demon fields.

This project builds via the included GitHub Actions workflow. Push the project files to GitHub, wait for the workflow to succeed, and download the Android64 or Windows artifact from the run's **Artifacts** section.
