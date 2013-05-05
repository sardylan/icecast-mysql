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


require_once($site_root . "/includes/functions.php");

define("MYSQL_CONN_ADDR", "localhost");
define("MYSQL_CONN_USER", "icecast");
define("MYSQL_CONN_PASSWD", "icecast");
define("MYSQL_CONN_DBNAME", "icecast");

define("ICECAST_PUBLIC_DOMAIN", "http://streaming.domain.ext:8000");

header("Cache-Control: no-cache, must-revalidate");

$sql_conn = new mysqli(MYSQL_CONN_ADDR, MYSQL_CONN_USER, MYSQL_CONN_PASSWD, MYSQL_CONN_DBNAME);

if ($sql_conn->connect_error)
    die("Connect Error (" . $mysqli->connect_errno . "): " . $mysqli->connect_error);

?>
