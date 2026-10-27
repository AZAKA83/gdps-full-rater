# Automatic GDPS detection patch

This patch removes the manual `server-url` setting and uses the current server returned by `ServerAPIEvents::getCurrentServer()` from `km7dev.server_api` (the Server API used by GDPS Switcher 2.0).

- The RATE button is hidden when the active server is the official `boomlings.com` server.
- On another active HTTPS server, the button is available and the mod builds the custom-rater endpoint URL from the active server URL.
- If the current server URL does not already end in `/database`, the Cvolton API directory is appended.
- Non-HTTPS rating submissions are blocked because they include the GDPS account token.

The server must still have `gdpsFullRaterGet.php`, `gdpsFullRaterSet.php`, and the schema installed. This patch only changes client-side detection/routing; it cannot deploy files to Glowhost. The source was not compiled locally in this environment, so GitHub Actions must confirm the build.
