<?php
/**
 * GDPS Full Rater: custom difficulty setter for Cvolton/GMDprivateServer.
 * Install this file in the Cvolton GDPS database/ directory beside rateGJStars20.php.
 * Requires server-addon/cvolton/schema.sql to be imported first.
 *
 * POST fields:
 *   accountID, gjp2 (normal Cvolton GDPS authentication), levelID, tier
 *   tier may be a supported tier key or "unrated" to remove the custom tier.
 *
 * This is a companion API for the matching Geode mod. It does not change
 * stock GD's built-in difficulty fields by itself; clients need the mod to
 * read gdpsFullRaterGet.php to display custom tiers.
 */
chdir(__DIR__);
require_once __DIR__ . '/../incl/lib/connection.php';
require_once __DIR__ . '/../incl/lib/GJPCheck.php';
require_once __DIR__ . '/../incl/lib/exploitPatch.php';
require_once __DIR__ . '/../incl/lib/mainLib.php';

header('Content-Type: application/json; charset=utf-8');

function gfr_response($ok, $code, $extra = array()) {
    echo json_encode(array_merge(array('ok' => $ok, 'code' => $code), $extra));
    exit;
}

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    gfr_response(false, 'POST_REQUIRED');
}

$levelRaw = isset($_POST['levelID']) ? $_POST['levelID'] : '';
$tierRaw = isset($_POST['tier']) ? $_POST['tier'] : '';
if (!is_scalar($levelRaw) || !ctype_digit((string)$levelRaw) || (int)$levelRaw < 1) {
    http_response_code(400);
    gfr_response(false, 'INVALID_LEVEL_ID');
}
$levelID = (int)$levelRaw;
$tierKey = strtolower(trim((string)$tierRaw));

$tiers = array(
    'auto' => 'Auto',
    'easy' => 'Easy',
    'normal' => 'Normal',
    'hard' => 'Hard',
    'harder' => 'Harder',
    'insane' => 'Insane',
    'easy-demon' => 'Easy Demon',
    'medium-demon' => 'Medium Demon',
    'hard-demon' => 'Hard Demon',
    'insane-demon' => 'Insane Demon',
    'extreme-demon' => 'Extreme Demon',
    'free-demon' => 'Free Demon',
    'peaceful-demon' => 'Peaceful Demon',
    'simple-demon' => 'Simple Demon',
    'casual-demon' => 'Casual Demon',
    'mild-demon' => 'Mild Demon',
    'moderate-demon' => 'Moderate Demon',
    'tricky-demon' => 'Tricky Demon',
    'tough-demon' => 'Tough Demon',
    'wild-demon' => 'Wild Demon',
    'cruel-demon' => 'Cruel Demon',
    'crazy-demon' => 'Crazy Demon',
    'bizarre-demon' => 'Bizarre Demon',
    'brutal-demon' => 'Brutal Demon',
    'supreme-demon' => 'Supreme Demon',
    'ultimate-demon' => 'Ultimate Demon',
    'legendary-demon' => 'Legendary Demon',
    'mythical-demon' => 'Mythical Demon',
    'infinite-demon' => 'Infinite Demon',
    'grandpa-demon' => 'Grandpa Demon'
);

// Authenticate the submitted account/token pair. Do not call getAccountIDOrDie()
// here: that helper uses relative includes for scripts under incl/levels, whereas
// this file is installed under database/.
$accountRaw = isset($_POST['accountID']) ? (string)$_POST['accountID'] : '';
$gjp2 = isset($_POST['gjp2']) && is_scalar($_POST['gjp2']) ? (string)$_POST['gjp2'] : '';
if ($accountRaw === '' || !ctype_digit($accountRaw) || (int)$accountRaw < 1 || $gjp2 === '') {
    http_response_code(401);
    gfr_response(false, 'AUTH_REQUIRED');
}
$accountID = (int)$accountRaw;
if (GeneratePass::isGJP2Valid($accountID, $gjp2) != 1) {
    http_response_code(401);
    gfr_response(false, 'INVALID_GJP2');
}
$gs = new mainLib();
if (!$gs->checkPermission($accountID, 'commandRate')) {
    http_response_code(403);
    gfr_response(false, 'MISSING_COMMAND_RATE_PERMISSION');
}

$levelCheck = $db->prepare('SELECT levelID FROM levels WHERE levelID = :levelID LIMIT 1');
$levelCheck->execute(array(':levelID' => $levelID));
if (!$levelCheck->fetchColumn()) {
    http_response_code(404);
    gfr_response(false, 'LEVEL_NOT_FOUND');
}

if ($tierKey === 'unrated') {
    $delete = $db->prepare('DELETE FROM gdps_full_rater_custom_difficulties WHERE levelID = :levelID');
    $delete->execute(array(':levelID' => $levelID));
    gfr_response(true, 'CUSTOM_TIER_REMOVED', array('levelID' => $levelID));
}

if (!isset($tiers[$tierKey])) {
    http_response_code(400);
    gfr_response(false, 'UNKNOWN_TIER');
}

$now = time();
$save = $db->prepare(
    'INSERT INTO gdps_full_rater_custom_difficulties (levelID, tierKey, tierName, ratedBy, updatedAt) '
    . 'VALUES (:levelID, :tierKey, :tierName, :ratedBy, :updatedAt) '
    . 'ON DUPLICATE KEY UPDATE tierKey = VALUES(tierKey), tierName = VALUES(tierName), '
    . 'ratedBy = VALUES(ratedBy), updatedAt = VALUES(updatedAt)'
);
$save->execute(array(
    ':levelID' => $levelID,
    ':tierKey' => $tierKey,
    ':tierName' => $tiers[$tierKey],
    ':ratedBy' => (int)$accountID,
    ':updatedAt' => $now
));

gfr_response(true, 'SAVED', array(
    'levelID' => $levelID,
    'tierKey' => $tierKey,
    'tierName' => $tiers[$tierKey],
    'updatedAt' => $now
));
?>
