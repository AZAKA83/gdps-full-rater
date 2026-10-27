# GDPS Full Rater

A Geode mod plus a Cvolton/GMDprivateServer add-on for storing one **custom tier** per online level. The client includes standard difficulty labels and extra tiers such as Supreme, Ultimate, Infinite, and Grandpa Demon. Custom tiers are stored separately from normal Geometry Dash star/difficulty fields.

## Build the Geode mod

Push these project files to GitHub. The included GitHub Actions workflow builds Android64 and Windows variants. The official build action exposes the artifact directory as `steps.build.outputs.build-output`; the workflow uploads that directory directly.

## GDPS URL configuration

In Geode settings, `GDPS Server URL` is the GDPS **base URL**, not the database path. For this instance it is:

`https://glowhost.us/fpdexekj`

Keep that exact base URL. The mod automatically appends `/database` when calling the custom endpoints. The mod setting doesn't need a trailing slash.

## Required server-side installation

A successful Geode build does NOT deploy PHP endpoints to Glowhost. For real persistent custom ratings on a compatible Cvolton/GMDprivateServer instance:

1. Back up your GDPS files and database.
2. Import `server-addon/cvolton/schema.sql` into the same MySQL/MariaDB database used by Cvolton.
3. Upload `server-addon/cvolton/gdpsFullRaterGet.php` and `server-addon/cvolton/gdpsFullRaterSet.php` into the GDPS `database/` directory beside `rateGJStars20.php`.
4. Those PHP files require core libraries from `../incl/lib/` relative to `database/`.
5. Test `https://YOUR-GDPS-BASE/database/gdpsFullRaterGet.php?levelID=1`. If the server returns 404, the PHP endpoint has not been deployed at that path.
6. The rating account must have Cvolton's `commandRate` permission.

If your Glowhost dashboard has no file-upload or SQL-import option, contact Glowhost support and ask them to install the two PHP files and import the schema for this project. GitHub cannot deploy server files or database changes on its own.

## In-game use

1. Sign in to your GDPS account inside Geometry Dash.
2. Open an uploaded/online level's info page and click the compact **RATE** button.
3. The mod reads the existing custom tier from the server.
4. Click **Next tier** to cycle through labels and **Save rating** to store the selection. **Remove custom rating** deletes the custom value.
5. The server checks the account's `commandRate` permission and GJP2 session token.

## What “real rating” means

Once the PHP add-on is deployed, custom tiers are server-backed and shared with other clients using this mod on the same GDPS. They do **not** automatically change standard stars, featured status, or built-in difficulty for vanilla clients; those remain controlled by the GDPS's normal rating workflow.
