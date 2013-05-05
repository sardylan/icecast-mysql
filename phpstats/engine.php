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
 */


header("Cache-Control: no-cache, must-revalidate");
header("Content-type: text/plain");

$page_path = "";
$cwd = getcwd();
$site_root = substr(getcwd(), 0, strlen($cwd) - strlen($page_path));

require_once($site_root . "/includes/head.php");

$page_result = array();

$interval_start = myget("start") . ":00";
$interval_stop = myget("stop") . ":59";

$get_mountpoints = explode(",", myget("mountpoints"));

$minimum_duration = 2;
$maximum_mostactive = 100;

$selected_mountpoints = "";
$mountpoints_count = 0;

foreach($get_mountpoints as $key) {
    if($mountpoints_count > 0)
        $selected_mountpoints .= ", ";
    $selected_mountpoints .= "{$key}";
    $mountpoints_count++;
}

$page_result["days"] = humanReadable((strtotime($interval_stop) - strtotime($interval_start) + 1) / (60*60*24), 2);
$page_result["start"] = $interval_start;
$page_result["stop"] = $interval_stop;

if($page_result["days"] <= 0) {
    die("ERROR");
}

if(myget("action") == "text") {

    $sql_query = "SELECT COUNT(id) AS listeners FROM stats WHERE start >= '{$interval_start}' AND stop <= '{$interval_stop}' AND mount IN ({$selected_mountpoints}) AND ip NOT IN (SELECT ip FROM ipblacklist)";
    error_log($sql_query);
    if($sql_result = $sql_conn->query($sql_query)) {
        if($sql_result->num_rows > 0) {
            $sql_data = $sql_result->fetch_array(MYSQLI_ASSOC);
            $page_result["listeners"] = $sql_data["listeners"];
        }
    }

    $sql_query = "SELECT MAX(duration) AS maxonlinetime, MIN(duration) AS minonlinetime FROM stats WHERE start >= '{$interval_start}' AND stop <= '{$interval_stop}' AND mount IN ({$selected_mountpoints}) AND ip NOT IN (SELECT ip FROM ipblacklist) AND duration > {$minimum_duration}";

    if($sql_result = $sql_conn->query($sql_query)) {
        if($sql_result->num_rows > 0) {
            $sql_data = $sql_result->fetch_array(MYSQLI_ASSOC);
            $page_result["maxonlinetime"] = gmdate("G:i:s", $sql_data["maxonlinetime"]);
            $page_result["minonlinetime"] = gmdate("G:i:s", $sql_data["minonlinetime"]);
        }
    }

    $sql_query = "SELECT duration FROM stats WHERE start >= '{$interval_start}' AND stop <= '{$interval_stop}' AND duration > {$minimum_duration} AND mount IN ({$selected_mountpoints}) AND ip NOT IN (SELECT ip FROM ipblacklist)";

    $sum_duration = 0;

    if($sql_result = $sql_conn->query($sql_query)) {
        if($sql_result->num_rows > 0) {
            while($sql_data = $sql_result->fetch_array(MYSQLI_ASSOC)) {
                $sum_duration += $sql_data["duration"];
            }
        }
    }

    $page_result["aveonlinetime"] = gmdate("G:i:s", (int) ($sum_duration / $sql_result->num_rows));

    printf("%s", json_encode($page_result));
}

if(myget("action") == "mountpoints") {
    $sql_query = "SELECT COUNT(tta.id) AS connections, ttb.mount FROM stats tta, mountpoints ttb WHERE tta.start >= '{$interval_start}' AND tta.stop <= '{$interval_stop}' AND tta.mount IN ({$selected_mountpoints}) AND ttb.id = tta.mount AND tta.ip NOT IN (SELECT ip FROM ipblacklist) GROUP BY mount ORDER BY connections DESC";

    echo "<ul>\n";

    if($sql_result = $sql_conn->query($sql_query)) {
        if($sql_result->num_rows > 0) {
            while($sql_data = $sql_result->fetch_array(MYSQLI_ASSOC)) {
                echo "    <li><a href=\"" . ICECAST_PUBLIC_DOMAIN . "{$sql_data["mount"]}\">{$sql_data["mount"]}</a> ({$sql_data["connections"]} connections)</li>\n";
            }
        }
    }

    echo "</ul>\n";
}

if(myget("action") == "table") {
    $sql_query = "SELECT tta.ip AS ip, ttb.mount AS mount, tta.start AS start, tta.stop AS stop, tta.agent AS agent, tta.duration AS duration FROM stats tta, mountpoints ttb WHERE tta.start >= '{$interval_start}' AND tta.stop <= '{$interval_stop}' AND tta.mount IN ({$selected_mountpoints}) AND tta.ip NOT IN (SELECT ip FROM ipblacklist) AND ttb.id = tta.mount ORDER BY duration DESC LIMIT 0, {$maximum_mostactive}";

    echo "<table>\n";
    echo "    <thead>\n";
    echo "        <tr>\n";
    echo "            <th class=\"mountpoints_ip\">IP</th>\n";
    echo "            <th class=\"mountpoints_mountpoint\">Mountpoint</th>\n";
    echo "            <th class=\"mountpoints_agent\">User Agent</th>\n";
    echo "            <th class=\"mountpoints_start\">Start time</th>\n";
    echo "            <th class=\"mountpoints_stop\">Stop time</th>\n";
    echo "            <th class=\"mountpoints_duration\">Duration</th>\n";
    echo "        </tr>\n";
    echo "    </thead>\n";
    echo "    <tbody>\n";

    if($sql_result = $sql_conn->query($sql_query)) {
        if($sql_result->num_rows > 0) {
            while($sql_data = $sql_result->fetch_array(MYSQLI_ASSOC)) {
                echo "        <tr>\n";
                echo "            <td class=\"mountpoints_ip\"><a href=\"http://www.geoiptool.com/en/?IP=" . $sql_data["ip"] . "\">{$sql_data["ip"]}</a></td>\n";
                echo "            <td class=\"mountpoints_mountpoint\">{$sql_data["mount"]}</td>\n";
                echo "            <td class=\"mountpoints_agent\">" . parseUserAgent($sql_data["agent"]) . "</td>\n";
                echo "            <td class=\"mountpoints_start\">" . splitDateTime($sql_data["start"]) . "</td>\n";
                echo "            <td class=\"mountpoints_stop\">" . splitDateTime($sql_data["stop"]) . "</td>\n";
                echo "            <td class=\"mountpoints_duration\">" . gmdate("G:i:s", $sql_data["duration"]) . "</td>\n";
                echo "        </tr>\n";
            }
        }
    }

    echo "    </tbody>\n";
    echo "</table>\n";
}

$sql_conn->close();

?>
