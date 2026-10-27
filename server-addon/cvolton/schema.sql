-- Import once into the same MySQL/MariaDB database as Cvolton/GMDprivateServer.
-- This table stores one custom difficulty tier per level for clients using
-- the GDPS Full Rater Geode mod. It does not alter the standard `levels` table.
CREATE TABLE IF NOT EXISTS `gdps_full_rater_custom_difficulties` (
  `levelID` INT(11) NOT NULL,
  `tierKey` VARCHAR(64) NOT NULL,
  `tierName` VARCHAR(64) NOT NULL,
  `ratedBy` INT(11) NOT NULL,
  `updatedAt` INT(11) NOT NULL,
  PRIMARY KEY (`levelID`),
  KEY `idx_gfr_ratedBy` (`ratedBy`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
