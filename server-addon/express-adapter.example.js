/**
 * Example Express routes for storing custom difficulty metadata.
 *
 * This is an adapter template, not a drop-in file for every GDPS core.
 * Your GDPS must supply:
 *   - requireLoggedIn: middleware which authenticates the current GDPS account
 *   - hasPermission(user, "commandRate"): checks the actual server permission
 *   - db: a SQLite-like object with exec/prepare (or adapt SQL calls for MySQL)
 *
 * Do NOT trust an account ID sent by the client. Use the account attached by
 * the server's own authentication middleware. Only the game/server owner should
 * adapt and deploy this with the actual GDPS core.
 */
const allowedTiers = new Set([
  "easy-demon", "medium-demon", "hard-demon", "insane-demon", "extreme-demon",
  "free-demon", "peaceful-demon", "simple-demon", "casual-demon", "mild-demon",
  "moderate-demon", "tricky-demon", "tough-demon", "wild-demon", "cruel-demon",
  "crazy-demon", "bizarre-demon", "brutal-demon", "supreme-demon", "ultimate-demon",
  "legendary-demon", "mythical-demon", "infinite-demon", "grandpa-demon"
]);

function registerFullRaterRoutes(app, { db, requireLoggedIn, hasPermission }) {
  db.exec(`CREATE TABLE IF NOT EXISTS gdps_custom_level_ratings (
    level_id INTEGER NOT NULL PRIMARY KEY,
    custom_tier TEXT NOT NULL,
    stars INTEGER NOT NULL DEFAULT 0,
    rated_by_account_id INTEGER NOT NULL,
    updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
  )`);

  app.get("/api/gdps-custom-rating/:levelId", (req, res) => {
    const levelId = Number(req.params.levelId);
    if (!Number.isSafeInteger(levelId) || levelId <= 0) {
      return res.status(400).json({ error: "invalid levelId" });
    }
    const row = db.prepare("SELECT level_id, custom_tier, stars, updated_at FROM gdps_custom_level_ratings WHERE level_id = ?").get(levelId);
    return res.json({ rating: row || null });
  });

  app.put("/api/gdps-custom-rating/:levelId", requireLoggedIn, (req, res) => {
    const user = req.user; // MUST be set by the GDPS's real auth middleware.
    if (!user || !Number.isSafeInteger(Number(user.accountID))) {
      return res.status(401).json({ error: "login required" });
    }
    if (!hasPermission(user, "commandRate")) {
      return res.status(403).json({ error: "missing commandRate permission" });
    }
    const levelId = Number(req.params.levelId);
    const tier = String(req.body?.custom_tier || "");
    const stars = Number(req.body?.stars ?? 0);
    if (!Number.isSafeInteger(levelId) || levelId <= 0) return res.status(400).json({ error: "invalid levelId" });
    if (!allowedTiers.has(tier)) return res.status(400).json({ error: "invalid custom_tier" });
    if (!Number.isInteger(stars) || stars < 0 || stars > 10) return res.status(400).json({ error: "stars must be 0..10" });

    db.prepare(`INSERT INTO gdps_custom_level_ratings (level_id, custom_tier, stars, rated_by_account_id, updated_at)
      VALUES (?, ?, ?, ?, CURRENT_TIMESTAMP)
      ON CONFLICT(level_id) DO UPDATE SET custom_tier=excluded.custom_tier, stars=excluded.stars,
      rated_by_account_id=excluded.rated_by_account_id, updated_at=CURRENT_TIMESTAMP`)
      .run(levelId, tier, stars, Number(user.accountID));
    return res.json({ ok: true, levelId, custom_tier: tier, stars });
  });
}

module.exports = { registerFullRaterRoutes };
