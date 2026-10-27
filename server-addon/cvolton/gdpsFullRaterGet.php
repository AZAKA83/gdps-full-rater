<?php
/** Public read API for the matching GDPS Full Rater Geode mod. */
chdir(__DIR__);
require_once __DIR__ . '/../incl/lib/connection.php';
require_once __DIR__ . '/../incl/lib/exploitPatch.php';

header('Content-Type: application/json; charset=utf-8');
$raw = isset($_GET['levelID']) ? $_GET['levelID'] : '';
if (!is_scalar($raw) || !ctype_digit((string)$raw) || (int)$raw < 1) {
    http_response_code(400);
    echo json_encode(array('ok' => false, 'code' => 'INVALID_LEVEL_ID'));
    exit;
}
$levelID = (int)$raw;
$query = $db->prepare('SELECT tierKey, tierName, updatedAt FROM gdps_full_rater_custom_difficulties WHERE levelID = :levelID LIMIT 1');
$query->execute(array(':levelID' => $levelID));
$row = $query->fetch(PDO::FETCH_ASSOC);
if (!$row) {
    echo json_encode(array('ok' => true, 'levelID' => $levelID, 'rating' => null));
    exit;
}
echo json_encode(array(
    'ok' => true,
    'levelID' => $levelID,
    'key' => $row['tierKey'],
    'name' => $row['tierName'],
    'updatedAt' => (int)$row['updatedAt']
));
?>
