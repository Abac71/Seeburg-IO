#!/usr/bin/php

<?php

require_once 'lib/sonos/sonos.class.php';
require_once 'lib/phil/msg.class.php';

$room = $argv[1];
$combo = $argv[2];
$tracks = include('track-config.php') ?: [];

if (!isset($tracks[$combo])) {
        \Phil\Msg::set_msg(\Phil\Msg::MSG_TRACK_NOT_FOUND);

        echo "Track {$combo} not found!\n\n";
        die(1);
}
else {
        \Phil\Msg::unset_msg(\Phil\Msg::MSG_TRACK_NOT_FOUND);
}

\Phil\Msg::set_msg(\Phil\Msg::MSG_SONOS_NOT_FOUND);
$sonos = \SonosPHPController::get_room_coordinator($room);

if ($sonos) {
        \Phil\Msg::unset_msg(\Phil\Msg::MSG_SONOS_NOT_FOUND);
}
else {
        die(1);
}

$playing = ($sonos->GetTransportInfo() == 'PLAYING');

if (!$playing) {
        $sonos->RemoveAllTracksFromQueue();
}

$sonos->AddSpotifyToQueue($tracks[$combo]);

if (!$playing) {
        $sonos->Play();
}
