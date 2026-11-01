<?php
/**
 * GDPS Full Rater: custom difficulty setter for Cvolton/GMDprivateServer.
 * Install into the server's database/ directory beside rateGJStars20.php.
 * Requires schema.sql to be imported first.
 * POST fields: accountID, gjp2, levelID, tier.
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
    'auto' => 'Auto', 'easy' => 'Easy', 'normal' => 'Normal',
    'hard' => 'Hard', 'harder' => 'Harder', 'insane' => 'Insane',
    'easy-demon' => 'Easy Demon', 'medium-demon' => 'Medium Demon',
    'hard-demon' => 'Hard Demon', 'insane-demon' => 'Insane Demon',
    'extreme-demon' => 'Extreme Demon'
);
$accountRaw = isset($_POST['accountID']) ? (string)$_POST['accountID'] : '';
$gjp2 = isset($_POST['gjp2']) && is_scalar($_POST['gjp2']) ? (string)$_POST['gjp2'] : '';
if ($accountRaw === '' || !ctype_digit($accountRaw) || (int)$accountRaw < 1 || $gjp2 === '') {
    http_response_code(401);
    gfr_response(false, 'AUTH_REQUIRED');
}
$accountID = (int)$accountRaw;
$authResult = GeneratePass::isGJP2Valid($accountID, $gjp2);
if ($authResult !== 1) {
    if ($authResult === -1) {
        http_response_code(429);
        gfr_response(false, 'AUTH_RATE_LIMITED');
    }
    if ($authResult === -2) {
        http_response_code(403);
        gfr_response(false, 'ACCOUNT_INACTIVE');
    }
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
