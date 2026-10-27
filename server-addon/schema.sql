-- Auxiliary table for custom difficulty tiers. Run this in the GDPS database.
-- It deliberately does not replace or alter the GDPS's normal level-rating columns.
CREATE TABLE IF NOT EXISTS gdps_custom_level_ratings (
    level_id INTEGER NOT NULL PRIMARY KEY,
    custom_tier VARCHAR(64) NOT NULL,
    stars INTEGER NOT NULL DEFAULT 0,
    rated_by_account_id INTEGER NOT NULL,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);
