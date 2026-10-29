# GDPS Full Rater

This Geode mod reads the active server from GDPS Switcher through `km7dev.server_api`.
It does not ask for a separate server URL. The current base URL stays whatever GDPS Switcher reports, for example `https://glowhost.us/fpdexekj`.

## Endpoint routing

The client first tries `BASE_URL/gdpsFullRaterGet.php` and `BASE_URL/gdpsFullRaterSet.php`. If a request returns HTTP 404, it tries `BASE_URL/database/gdpsFullRaterGet.php` or `BASE_URL/database/gdpsFullRaterSet.php` once. This handles hosts that expose custom scripts at the root and hosts that expose the `/database` path.

A client-side build cannot create these endpoints on a hosted server. For persistent custom ratings, the GDPS must install/expose `server-addon/cvolton/gdpsFullRaterGet.php` and `gdpsFullRaterSet.php` and import `server-addon/cvolton/schema.sql` into the same database as the GDPS. The PHP code expects the Cvolton/GMDprivateServer library layout; other server cores need an adapter.

For the Glowhost instance `https://glowhost.us/fpdexekj`, the mod keeps that base URL and appends endpoint filenames itself. Do not type `/database` into GDPS Switcher just for this mod. If both endpoint routes return 404, Glowhost must deploy or expose the custom endpoint; rebuilding the `.geode` again won't fix a missing server route.

## Build

The `.github/workflows/build.yml` workflow builds the Windows and Android64 variants on GitHub Actions. Download the artifact only after the relevant build succeeds.

## In-game use

Open an uploaded online level on a GDPS selected through GDPS Switcher, click the compact RATE button, select a tier with **Next tier**, and choose **Save rating**. The server add-on validates the Cvolton GJP2 token and `commandRate` permission. Custom tiers are stored separately from standard stars, built-in difficulty, and featured status.

Made By AZAKA83  
