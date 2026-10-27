# Cvolton GMDprivateServer server add-on

Stores one custom difficulty tier per online level in a separate database table. It does not change standard stars, featured state, or the built-in difficulty.

## Install on a Cvolton server

1. Back up the GDPS files and database.
2. Import `schema.sql` into the same MySQL/MariaDB database the GDPS uses.
3. Upload `gdpsFullRaterGet.php` and `gdpsFullRaterSet.php` to `database/`, next to `rateGJStars20.php`.
4. The scripts load core library files from `../incl/lib/`, matching the standard Cvolton folder layout.
5. Configure the Geode mod with the **base URL** of the GDPS, for example `https://glowhost.us/fpdexekj`. The mod appends `/database` automatically.
6. Test the read endpoint at `https://YOUR-GDPS-BASE/database/gdpsFullRaterGet.php?levelID=1`. It should return JSON. A 404 means the add-on file is not installed at that path.
7. Grant your account the Cvolton `commandRate` permission.

The setter accepts whitelisted tier keys only and validates the account's GJP2 token. The custom tier is for compatible clients using this mod. Use Cvolton's normal rating workflow for standard star/difficulty changes.

## Hosted Glowhost projects

Pushing this repository to GitHub does not put PHP files on the Glowhost server. If the dashboard does not provide server-file editing/upload and SQL import for your instance, contact Glowhost support and ask them to deploy these two PHP files and import `schema.sql` for your GDPS. Do not send them your password or account token.
