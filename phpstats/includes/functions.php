<?php

/*
 * PHP Site for Icecast MySQL Stats
 * Copyright (C) 2013  Luca Cireddu
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * You can also find an on-line copy of this license at:
 * http://www.gnu.org/licenses/gpl-2.0.html
 *
 * Created and mantained by: Luca Cireddu
 *                           sardylan@gmail.com
 *                           http://www.lucacireddu.it
 *
 * Many of these functions are created by Paolo Cortis (paolo.cortis@gmail.com)
 *
 */


function mysql2jsDate($input)
{
    list($in_date, $in_time) = explode(" ", $input);
    $out_date = explode("-", $in_date);
    $out_time = explode(":", $in_time);

    return (int) $out_date[0] . ", " . (int) ($out_date[1] - 1) . ", " . (int) $out_date[2] . ", " . (int) $out_time[0] . ", " . (int) $out_time[1];
    //. ", " . (int) $out_time[2];
}

function getDirs($dir)
{
    $files = scandir($dir);

    foreach($files as $i => $value) {
        if(substr($value, 0, 1) == ".")
            unset($files[$i]);
        elseif(!is_dir($dir.$value))
            unset($files[$i]);
        }

    return array_values($files);
}

function getFiles($dir, $type = "")
{
    $files = scandir($dir);
    $type_len = strlen($type);

    foreach($files as $i => $value) {
        if(substr($value, 0, 1) == ".")
            unset($files[$i]);
        elseif(is_dir($dir.$value))
            unset($files[$i]);
        elseif(substr($value, -1, 1) == "~")
            unset($files[$i]);
        elseif($type_len > 0 && substr($value, -$type_len, $type_len) != $type)
            unset($files[$i]);
    }

    return array_values($files);
}

function str_esc($input)
{
    if(get_magic_quotes_gpc())
        return $input;
    else
        return addslashes($input);
}

function file_pos($file, $search, $direction = 1)
{
    $rowCount = count($file);
    if($direction > 0) {
        for($i = 0; $i < $rowCount; $i++) {
            if(starts_with($file[$i], "$search")) {
                return $i;
            }
        }
    } else {
        for($i = $rowCount; $i > 0; $i--) {
            if(starts_with($file[$i], "$search")) {
                return $i;
            }
        }
    }
    return -1;
}

function file_contains($file, $search, $direction = 1)
{
    return (file_pos($file, $search, $direction) >= 0);
}

function myget($key)
{
    if(isset($_GET[$key])) {
        return str_esc($_GET[$key]);
    } else {
        return "";
    }
}

function starts_with($string, $search)
{
    return substr($string, 0, strlen($search)) == "{$search}";
}

function word_count($stringa)
{
    return count(explode(" ", $stringa));
}

function orderBy($data, $field) {
    $code = "return strnatcmp(\$a['$field'], \$b['$field']);";
    rsort($data, create_function('$a,$b', $code));
    return $data;
}

function divideByThousand($value)
{
    if($value == 0)
        return "0";

    return (int) ($value / 1000) . "K";
}

function humanReadable($value, $decimals = 0)
{
    return number_format($value, $decimals);
}

function printVar($input) {
    printf("<pre>\n");
    foreach(explode("\n", print_r($input, true)) as $row) {
        printf("%s\n", $row);
    }
    printf("</pre>\n");
}


function parseUserAgent($input = "")
{
    $ret = "";

    if(strlen($input) > 0) {
        $ret = $input;
    }

    return $ret;
}

function splitDateTime($input = "")
{
    $ret = "";

    if(strlen($input) > 0) {
        list($year, $month, $day, $hour, $minutes, $seconds) = preg_split('/\D+/', $input);
        $ret = "<span class=\"yearsmall\">{$year}-{$month}-{$day}</span><br />{$hour}:{$minutes}:{$seconds}";
    }

    return $ret;
}
?>
