# Server-side custom rating adapter

The table and Express route example store custom demon tiers separately from the normal Geometry Dash star/difficulty fields. This is necessary because the standard GDPS `!rate` command normally accepts only the built-in categories (Auto, Easy, Normal, Hard, Harder, Insane, Demon).

## Before using

1. Back up your GDPS database.
2. Identify the server core and its authentication/permission middleware.
3. Adapt `express-adapter.example.js` so `req.user` comes from the core's verified login session, and `hasPermission` uses the core's real roles.
4. Adapt the SQL if the core uses MySQL rather than SQLite.
5. Add a client request in the Geode mod to GET the custom rating and PUT a rating after the permission check.
6. Test with a non-admin account first; then test the owner account.

The example is not safe to deploy unchanged because each GDPS exposes different account/session objects. Never trust a username or account ID submitted by the mod, and never put a database password or server secret into a `.geode` file.
