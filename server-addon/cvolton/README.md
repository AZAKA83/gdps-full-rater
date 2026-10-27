# Cvolton GMDprivateServer adapter (custom tiers)

This adapter is targeted at the **Cvolton/GMDprivateServer** PHP + MySQL core.
It gives the companion Geode mod a persistent server-side value for custom
level difficulty tiers. It does not modify the original Cvolton files.

## Important behavior

- It uses Cvolton's `GJPCheck::getAccountIDOrDie()` session-token validation.
- The write endpoint checks Cvolton's `commandRate` permission.
- Only the explicit tier whitelist is accepted.
- The read endpoint exposes only the level's custom tier name/key and update time.
- The table stores one custom tier per level, shared by players who use the
  matching mod and connect to this GDPS.
- This custom tier is **not automatically shown in vanilla Geometry Dash**.
  The matching Geode client must call the read endpoint and render the tier.
- The adapter does not automatically set built-in stars, feature status, or
  coin verification. Use the ordinary Cvolton rating workflow for those fields.

## Installation

1. Back up your GDPS files and database first.
2. Import `schema.sql` into the same MySQL/MariaDB database Cvolton uses.
3. Copy `gdpsFullRaterSet.php` and `gdpsFullRaterGet.php` into the **same Cvolton API directory as `rateGJStars20.php`** (often the `database/` directory),
   beside files such as `rateGJStars20.php` and `rateGJDemon21.php`.
4. Keep PHP errors out of public responses and use HTTPS.
5. Test the read endpoint in a browser:
   `https://YOUR-GDPS-DOMAIN/gdpsFullRaterGet.php?levelID=1`
6. In Geode > Mods > GDPS Full Rater settings, configure the HTTPS URL of the directory that contains these PHP files (often ending in `/database`), without a trailing slash. The mod POSTs `accountID`, `gjp2`, `levelID`, and `tier` to `gdpsFullRaterSet.php` and GETs `gdpsFullRaterGet.php?levelID=...` to display the saved tier.

## POST fields for `gdpsFullRaterSet.php`

- `accountID`: account ID; it is validated along with the session token.
- `gjp2`: the account's usual GDPS login token (`GJPCheck` handles validation).
- `levelID`: numeric level ID.
- `tier`: one of the keys in `../difficulties.json`, for example `grandpa-demon`.
  Use `unrated` to remove the custom tier.

The account must have Cvolton's `commandRate` permission. Do not put account
passwords, GJP tokens, or server secrets into a public repository or hard-code
them inside mod source. A finished client implementation must use the game
session's token in memory and send it only over HTTPS to the configured GDPS.

## Notes / limits

This is a separate custom-tier record. To make the exact custom tier show up in
Cvolton's stock web dashboard or unmodified GD clients, those components would
also need changes. It is not an official RobTop rating and is only authoritative
on the GDPS where this addon is installed.
