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


$page_path = "";
$cwd = getcwd();
$site_root = substr(getcwd(), 0, strlen($cwd) - strlen($page_path));

require_once($site_root . "/includes/head.php");

?>
<html>

    <head>
        <title>Icecast Stats</title>
        <meta http-equiv="content-type" content="text/html; charset=utf-8">
        <script type="text/javascript" src="js/jquery/jquery.min.js"></script>
        <script type="text/javascript" src="js/jquery-ui/jquery.ui.core.min.js"></script>
        <script type="text/javascript" src="js/jquery-ui/jquery.ui.widget.min.js"></script>
        <script type="text/javascript" src="js/jquery-ui/jquery.ui.mouse.min.js"></script>
        <script type="text/javascript" src="js/jquery-ui/jquery.ui.slider.min.js"></script>
        <script type="text/javascript" src="js/jquery-ui/jquery.ui.datepicker.min.js"></script>
        <script type="text/javascript" src="js/jquery-ui/i18n/jquery.ui.datepicker-it.js"></script>
        <script type="text/javascript" src="js/jquery-ui-timepicker/jquery-ui-timepicker-addon.js"></script>
        <script type="text/javascript" src="js/jquery-powertip/jquery.powertip.min.js"></script>
        <script type="text/javascript" src="js/clock.js"></script>
        <script type="text/javascript" src="js/statusbar.js"></script>
        <script type="text/javascript" src="js/form.php"></script>
        <script type="text/javascript" src="js/hover.js"></script>
        <link rel="stylesheet" type="text/css" href="styles/jquery-ui/jquery.ui.all.css" />
        <link rel="stylesheet" type="text/css" href="styles/jquery-ui-timepicker/timepicker.css" />
        <link rel="stylesheet" type="text/css" href="styles/jquery-powertip/jquery.powertip.css" />
        <link rel="stylesheet" type="text/css" href="styles/default.css" />
        <link rel="stylesheet" type="text/css" href="styles/content.css" />
        <link rel="stylesheet" type="text/css" href="styles/table.css" />
    </head>

    <body>
        <div id="page">
            <div id="header">
                <div id="logo"></div>
                <div id="title"></div>
            </div>
            <div id="searchmounts">
                <form>
                    <div id="searchmounts_label">Available mountpoints</div>
                    <div class="searchmounts_list">
                        <select id="searchmounts_select" name="searchmounts_select" multiple>
<?php

$sql_query = "SELECT * FROM mountpoints";

if($sql_result = $sql_conn->query($sql_query)) {
    if($sql_result->num_rows > 0) {
        while($sql_data = $sql_result->fetch_array(MYSQLI_ASSOC)) {
            echo "                            <option value=\"{$sql_data["id"]}\" selected=\"selected\">{$sql_data["mount"]}</option>\n";
        }
    }
}

?>
                        </select>
                    </div>
                    <div class="searchmounts_button"><input id="searchmounts_selectall" name="searchmounts_selectall" type="button" value="Select all" /></div>
                    <div class="searchmounts_button"><input id="searchmounts_deselectall" name="searchmounts_deselectall" type="button" value="Deselect all" /></div>
                </form>
            </div>
            <div id="searchseparator">
            </div>
            <div id="search">
                <form>
                    <div id="search_label">Choose interval to search</div>
                    <div class="search_button"><input id="search_24h" name="search_24h" type="button" value="Last 24 hours" /></div>
                    <div class="search_button"><input id="search_7d" name="search_7d" type="button" value="Last 7 days" /></div>
                    <div class="search_button"><input id="search_30d" name="search_30d" type="button" value="Last month" /></div>
                    <div class="search_button"><input id="search_60d" name="search_60d" type="button" value="Last 2 month" /></div>
                    <div class="search_button"><input id="search_year" name="search_year" type="button" value="Last year" /></div>
                    <div class="search_button"><input id="search_manual" name="search_manual" type="button" value="Manual interval" /></div>
                    <div id="search_interval">
                        <input class="interval" id="search_start" name="search_start" type="input" />
                        <input class="interval" id="search_stop" name="search_stop" type="input" />
                    </div>
                </form>
            </div>
            <div id="content">
                <h1>Activities from <span id="res_start"></span> to <span id="res_stop"></span></h1>
                <div id="resp_text">
                    <h2>Summary</h2>
                    <p>Total days: <span class="res_summary" id="res_days"></span><br />
                        Total listeners: <span class="res_summary" id="res_listeners"></span><br />
                        Max on-line time: <span class="res_summary" id="res_maxonlinetime"></span><br />
                        Min on-line time: <span class="res_summary" id="res_minonlinetime"></span><br />
                        Average on-line time: <span class="res_summary" id="res_aveonlinetime"></span></p>
                    <h2>Mountpoints</h2>
                    <div id="resp_mountpoints_list"></div>
                </div>
                <div id="resp_graph">
                    <img id="img_listeners" src="images/loading.png" alt="Listeners" title="Listeners" />
                </div>
                <div class="divclear"></div>
                <div id="resp_table">
                    <h2>Most active listeners</h2>
                    <div id="resp_table_list"></div>
                </div>
            </div>
            <div id="rulerspace">
            </div>
        </div>
        <div id="footer">
            <div class="footer-element-left" id="footer_info"><span id="value_info"></span></div>
            <div class="footer-element-right" id="footer_clock"><span id="value_clock">Loading...</span></div>
            <div class="footer-element-right" id="footer_online">Online: <span id="value_online">NI</span></div>
            <div class="footer-element-right" id="footer_mounts">Available mountpoints on DB: <span id="value_mounts"></span></div>
            <div class="footer-element-right" id="footer_records">Total listeners in DB: <span id="value_records"></span></div>
        </div>
    </body>

</html>
<?php

$sql_conn->close();

?>