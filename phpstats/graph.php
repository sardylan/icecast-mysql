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
// header("Content-type: text/plain");

$page_path = "";
$cwd = getcwd();
$site_root = substr(getcwd(), 0, strlen($cwd) - strlen($page_path));

$jpgraph_include_path = "/includes/jpgraph-3.5.0b1/src";

require_once($site_root . "/includes/head.php");
require_once($site_root . $jpgraph_include_path . "/jpgraph.php");
require_once($site_root . $jpgraph_include_path . "/jpgraph_line.php");
require_once($site_root . $jpgraph_include_path . "/jpgraph_ttf.inc.php");
require_once($site_root . $jpgraph_include_path . "/jpgraph_utils.inc.php");

$page_result = array();

$interval_start = myget("start") . ":00";
$interval_stop = myget("stop") . ":59";

$get_mountpoints = explode(",", myget("mountpoints"));

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

$page["data"] = array();
$page["labels"] = array();

if($page_result["days"] < 2) {
    $page["title"] = "Per-hour listeners";
    $page["labelstype"] = "hour";
    $page["datainterval"] = 60*60;
    $page["labelinterval"] = 4;

    $max = (int) ((strtotime($interval_stop) - strtotime($interval_start) + 1) / $page["datainterval"]);

    for($i=0; $i<$max; $i++) {
        array_push($page["labels"], strftime("%H", strtotime($interval_start) + 1 + ($page["datainterval"] * $i)));
    }

} else if($page_result["days"] < 30) {

    $page["title"] = "Per-day listeners";
    $page["labelstype"] = "day";
    $page["datainterval"] = 24*60*60;
    $page["labelinterval"] = 1;

    $max = (int) ((strtotime($interval_stop) - strtotime($interval_start) + 1) / $page["datainterval"]);

    for($i=0; $i<$max; $i++) {
        array_push($page["labels"], strftime("%d", strtotime($interval_start) + 1 + (60 * 60 * 24 * $i)));
    }

} else {

    $page["title"] = "Per-month listeners";
    $page["labelstype"] = "month";
    $page["datainterval"] = 30*24*60*60;
    $page["labelinterval"] = 1;

    $max = (int) ((strtotime($interval_stop) - strtotime($interval_start) + 1) / $page["datainterval"]);

    for($i=0; $i<$max; $i++) {
        array_push($page["labels"], strftime("%m", strtotime($interval_start) + 1 + ($page["datainterval"] * $i)));
    }

}

if(myget("action") == "pergraph") {

    for($i=0; $i<$max; $i++) {
        $query_start = strftime("%Y-%m-%d %H:%I:%S", strtotime($interval_start) + 1 + ($page["datainterval"] * $i));
        $query_stop = strftime("%Y-%m-%d %H:%I:%S", strtotime($interval_start) + 1 + ($page["datainterval"] * ($i + 1)));

        $sql_query = "SELECT COUNT(id) AS listeners FROM stats WHERE start >= '{$query_start}' AND stop <= '{$query_stop}' AND mount IN ({$selected_mountpoints}) AND ip NOT IN (SELECT ip FROM ipblacklist)";

        if($sql_result = $sql_conn->query($sql_query)) {
            if($sql_result->num_rows > 0) {
                $sql_data = $sql_result->fetch_array(MYSQLI_ASSOC);
                array_push($page["data"], $sql_data["listeners"]);
            }
        }
    }

    $graph = new Graph(600, 250);
    $graph->SetScale("textlin");

    $theme_class = new UniversalTheme;

    $graph->SetTheme($theme_class);
    $graph->img->SetAntiAliasing(false);
    $graph->title->Set($page["title"]);
    $graph->SetBox(false);

    $graph->img->SetAntiAliasing();

    $graph->yaxis->HideZeroLabel();
    $graph->yaxis->HideLine(false);
    $graph->yaxis->HideTicks(false, false);

    $graph->xgrid->Show();
    $graph->xgrid->SetLineStyle("solid");
    $graph->xaxis->SetTickLabels($page["labels"]);
    $graph->xaxis->SetTextLabelInterval($page["labelinterval"]);
    $graph->xgrid->SetColor("#E3E3E3");

    $p1 = new LinePlot($page["data"]);
    $p1->SetColor("#6495ED");
    $p1->SetLegend("Connections");

    $graph->Add($p1);
    $graph->legend->SetFrameWeight(1);
    $graph->Stroke();

}

$sql_conn->close();

?>