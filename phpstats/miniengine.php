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

if(myget("action") == "statusbar") {

    $sql_query = "SELECT COUNT(id) AS records FROM stats";

    if($sql_result = $sql_conn->query($sql_query)) {
        if($sql_result->num_rows > 0) {
            $sql_data = $sql_result->fetch_array(MYSQLI_ASSOC);
            $page_result["records"] = $sql_data["records"];
        }
    }

    $sql_query = "SELECT COUNT(id) AS mounts FROM mountpoints";

    if($sql_result = $sql_conn->query($sql_query)) {
        if($sql_result->num_rows > 0) {
            $sql_data = $sql_result->fetch_array(MYSQLI_ASSOC);
            $page_result["mounts"] = $sql_data["mounts"];
        }
    }

    $sql_query = "SELECT COUNT(id) AS online FROM online";

    if($sql_result = $sql_conn->query($sql_query)) {
        if($sql_result->num_rows > 0) {
            $sql_data = $sql_result->fetch_array(MYSQLI_ASSOC);
            $page_result["online"] = $sql_data["online"];
        }
    }

    printf("%s", json_encode($page_result));
}

$sql_conn->close();

?>