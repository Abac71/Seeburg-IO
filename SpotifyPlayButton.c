<?php

$tracks = require('tracks.php');

usort($tracks, function($a, $b) {
        if (isset($a['christmas']) && !isset($b['christmas'])) {
                return 1;
        }
        if (isset($b['christmas']) && !isset($a['christmas'])) {
                return -1;
        }

        if ($a['artist'] == $b['artist']) {
                return strcmp($a['track'], $b['track']);
        }

        return strcmp($a['artist'], $b['artist']);
});

$tracks = array_values($tracks);

$track_draw = new ImagickDraw();
$track_draw->setFont('fonts/ARIALBD.TTF');
$track_draw->setFontSize(30);
$track_draw->setFontWeight(900);
$track_draw->setTextAntialias(true);
$track_draw->setTextEncoding('UTF-8');

$artist_draw = new ImagickDraw();
$artist_draw->setFont('fonts/ARIALBD.TTF');
$artist_draw->setFontSize(20);
$artist_draw->setFontWeight(900);
$artist_draw->setTextAntialias(true);
$artist_draw->setTextEncoding('UTF-8');

$flip_flop = 0;
$letter_order = ['J', 'K', 'G', 'H', 'A', 'B', 'C', 'D', 'E', 'F'];
$letter = 0;
$number = 1;

$config = [];

for ($i = 0; $i < count($tracks); $i += 2) {
        // The track
        $track = $tracks[$i];
        $track['track'] = explode(' - ', $track['track'])[0];

        $config["{$letter_order[$letter]}{$number}"] = $track['id'];

        // Load base
        if (!$flip_flop) {
                $img = new Imagick('in/jukecard-orange.png');
        }
        else {
                $img = new Imagick('in/jukecard-green.png');
        }

        // Calc position of track
        $metrics = $img->queryFontMetrics($track_draw, $track['track'], false);
        $text_x = floor(336 - ($metrics['textWidth'] / 2));
        $text_y = 52;

        // Add track
        $img->annotateImage($track_draw, $text_x, $text_y, 0, $track['track']);

        // Calc position of artist
        $metrics = $img->queryFontMetrics($artist_draw, $track['artist'], false);
        $text_x = floor(336 - ($metrics['textWidth'] / 2));
        $text_y = 101;

        // Add artist
        $img->annotateImage($artist_draw, $text_x, $text_y, 0, $track['artist']);

        // The second track
        if (isset($tracks[$i + 1])) {
                $track = $tracks[$i + 1];
                $track['track'] = explode(' - ', $track['track'])[0];

                // Calc position of track
                $metrics = $img->queryFontMetrics($track_draw, $track['track'], false);
                $text_x = floor(336 - ($metrics['textWidth'] / 2));
                $text_y = 188;

                // Add track
                $img->annotateImage($track_draw, $text_x, $text_y, 0, $track['track']);

                // Calc position of artist
                $metrics = $img->queryFontMetrics($artist_draw, $track['artist'], false);
                $text_x = floor(336 - ($metrics['textWidth'] / 2));
                $text_y = 133;

                // Add artist
                $img->annotateImage($artist_draw, $text_x, $text_y, 0, $track['artist']);
        }

        $filename = $letter_order[$letter].$number.'.png';
        $img->writeImage("out/{$filename}");

        $num = $number+1;

        if ($num == 10) {
                $num = 0;
        }

        $config["{$letter_order[$letter]}{$num}"] = $track['id'];

        if ($number == 9) {
                $number = 1;
                $letter++;
        }
        else {
                $number += 2;
        }

        $flip_flop = !$flip_flop;
}
