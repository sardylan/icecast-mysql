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


header("Content-type: text/javascript");

$page_path = "/js";
$cwd = getcwd();
$site_root = substr(getcwd(), 0, strlen($cwd) - strlen($page_path));

require_once($site_root . "/includes/head.php");

$sql_query = "SELECT MIN(start) AS start, MAX(stop) AS stop, MAX(duration) AS duration FROM stats";

if($sql_result = $sql_conn->query($sql_query)) {
    if($sql_result->num_rows > 0) {
        $sql_data = $sql_result->fetch_array(MYSQLI_ASSOC);
        $page["start"] = $sql_data["start"];
        $page["stop"] = $sql_data["stop"];
        $page["duration"] = $sql_data["duration"];
    }
}

$sql_conn->close();

?>
function runsearch()
{
    $("#content").hide();

    if($("#searchmounts_select").val() != null) {

        $("#value_info").html("Searching activities from " + $("#search_start").val() + " to " + $("#search_stop").val() + " about " + $("#searchmounts_select").val().length + " mountpoints");

        $.ajax({
            type: "GET",
            url: "engine.php",
            data: "action=text&start=" + $("#search_start").val() + "&stop=" + $("#search_stop").val() + "&mountpoints=" + $("#searchmounts_select").val().join(),
            success: function(response) {
                var objResult = jQuery.parseJSON(response);
                if(objResult.listeners > 0) {
                    $("#content").show();
                    $("#img_listeners").attr("src", "graph.php?action=pergraph&start=" + $("#search_start").val() + "&stop=" + $("#search_stop").val() + "&mountpoints=" + $("#searchmounts_select").val().join());
                    $("#value_info").html("Search ended with " + objResult.listeners + " listeners");
                    $("#res_start").html(objResult.start);
                    $("#res_stop").html(objResult.stop);
                    $("#res_days").html(objResult.days);
                    $("#res_listeners").html(objResult.listeners);
                    $("#res_maxonlinetime").html(objResult.maxonlinetime);
                    $("#res_minonlinetime").html(objResult.minonlinetime);
                    $("#res_aveonlinetime").html(objResult.aveonlinetime);
                } else {
                    alert("No listeners on the selected period");
                    $("#value_info").html("Search ended with no result");
                }

                $.ajax({
                    type: "GET",
                    url: "engine.php",
                    data: "action=mountpoints&start=" + $("#search_start").val() + "&stop=" + $("#search_stop").val() + "&mountpoints=" + $("#searchmounts_select").val().join(),
                    success: function(response) {
                        $("#resp_mountpoints_list").html(response);
                    }
                });

                $.ajax({
                    type: "GET",
                    url: "engine.php",
                    data: "action=table&start=" + $("#search_start").val() + "&stop=" + $("#search_stop").val() + "&mountpoints=" + $("#searchmounts_select").val().join(),
                    success: function(response) {
                        $("#resp_table_list").html(response);
                    }
                });

            }
        });
    } else {
        $("#value_info").html("No mountpoints selected");
        alert("Please select at least one mountpoint");
    }
}

$(document).ready(function() {
    $("#content").hide();

    $("#searchmounts_selectall").click(function() {
        $("#searchmounts_select option").attr("selected","selected");
    });

    $("#searchmounts_deselectall").click(function() {
        $("#searchmounts_select option:selected").prop("selected", false);
    });

    $("#search_start").datetimepicker({
        dateFormat: "yy-mm-dd",
        buttonText: "Choose",
        showOn: "button",
        closeText: "Done",
        timeFormat: "hh:mm",
        minDate: new Date(<?php echo mysql2jsDate($page["start"]); ?>),
        maxDate: new Date(<?php echo mysql2jsDate($page["stop"]); ?>),
        separator: " ",
        hourGrid: 4,
        minuteGrid: 10,
        timeText: "When",
        hourText: "Hours",
        minuteText: "Minutes",
        currentText: "Now"
    });

    $("#search_stop").datetimepicker({
        dateFormat: "yy-mm-dd",
        buttonText: "Choose",
        showOn: "button",
        closeText: "Done",
        timeFormat: "hh:mm",
        minDate: new Date(<?php echo mysql2jsDate($sql_data["start"]); ?>),
        maxDate: new Date(<?php echo mysql2jsDate($sql_data["stop"]); ?>),
        separator: " ",
        hourGrid: 4,
        minuteGrid: 10,
        timeText: "When",
        hourText: "Hours",
        minuteText: "Minutes",
        currentText: "Now"
    });

    $("#search_start").datetimepicker("setDate", new Date(<?php echo mysql2jsDate($sql_data["start"]); ?>));
    $("#search_stop").datetimepicker("setDate", new Date(<?php echo mysql2jsDate($sql_data["stop"]); ?>));

    $("#search_24h").click(function() {
        var date_stop = new Date();
        var date_start = new Date(date_stop.valueOf() - 1000 * 3600 * 24);

        if(date_start < new Date(<?php echo mysql2jsDate($sql_data["start"]); ?>))
            date_start = new Date(<?php echo mysql2jsDate($sql_data["start"]); ?>);

        if(date_stop > new Date(<?php echo mysql2jsDate($sql_data["stop"]); ?>))
            date_stop = new Date(<?php echo mysql2jsDate($sql_data["stop"]); ?>);

        $("#search_start").datetimepicker("setDate", date_start);
        $("#search_stop").datetimepicker("setDate", date_stop);

        runsearch();
    });

    $("#search_7d").click(function() {
        var date_stop = new Date();
        var date_start = new Date(date_stop.valueOf() - 1000 * 3600 * 24 * 7);

        if(date_start < new Date(<?php echo mysql2jsDate($sql_data["start"]); ?>))
            date_start = new Date(<?php echo mysql2jsDate($sql_data["start"]); ?>);

        if(date_stop > new Date(<?php echo mysql2jsDate($sql_data["stop"]); ?>))
            date_stop = new Date(<?php echo mysql2jsDate($sql_data["stop"]); ?>);

        $("#search_start").datetimepicker("setDate", date_start);
        $("#search_stop").datetimepicker("setDate", date_stop);

        runsearch();
    });

    $("#search_30d").click(function() {
        var date_stop = new Date();
        var date_start = new Date(date_stop.valueOf() - 1000 * 3600 * 24 * 30);

        if(date_start < new Date(<?php echo mysql2jsDate($sql_data["start"]); ?>))
            date_start = new Date(<?php echo mysql2jsDate($sql_data["start"]); ?>);

        if(date_stop > new Date(<?php echo mysql2jsDate($sql_data["stop"]); ?>))
            date_stop = new Date(<?php echo mysql2jsDate($sql_data["stop"]); ?>);

        $("#search_start").datetimepicker("setDate", date_start);
        $("#search_stop").datetimepicker("setDate", date_stop);

        runsearch();
    });

    $("#search_60d").click(function() {
        var date_stop = new Date();
        var date_start = new Date(date_stop.valueOf() - 1000 * 3600 * 24 * 60);

        if(date_start < new Date(<?php echo mysql2jsDate($sql_data["start"]); ?>))
            date_start = new Date(<?php echo mysql2jsDate($sql_data["start"]); ?>);

        if(date_stop > new Date(<?php echo mysql2jsDate($sql_data["stop"]); ?>))
            date_stop = new Date(<?php echo mysql2jsDate($sql_data["stop"]); ?>);

        $("#search_start").datetimepicker("setDate", date_start);
        $("#search_stop").datetimepicker("setDate", date_stop);

        runsearch();
    });

    $("#search_year").click(function() {
        var date_stop = new Date();
        var date_start = new Date(date_stop.valueOf() - 1000 * 3600 * 24 * 365);

        if(date_start < new Date(<?php echo mysql2jsDate($sql_data["start"]); ?>))
            date_start = new Date(<?php echo mysql2jsDate($sql_data["start"]); ?>);

        if(date_stop > new Date(<?php echo mysql2jsDate($sql_data["stop"]); ?>))
            date_stop = new Date(<?php echo mysql2jsDate($sql_data["stop"]); ?>);

        $("#search_start").datetimepicker("setDate", date_start);
        $("#search_stop").datetimepicker("setDate", date_stop);

        runsearch();
    });

    $("#search_manual").click(function() {
        var date_start = $("#search_start").datetimepicker("getDate");
        var date_stop = $("#search_stop").datetimepicker("getDate");

        if(date_start < new Date(<?php echo mysql2jsDate($sql_data["start"]); ?>))
            date_start = new Date(<?php echo mysql2jsDate($sql_data["start"]); ?>);

        if(date_stop > new Date(<?php echo mysql2jsDate($sql_data["stop"]); ?>))
            date_stop = new Date(<?php echo mysql2jsDate($sql_data["stop"]); ?>);

        $("#search_start").datetimepicker("setDate", date_start);
        $("#search_stop").datetimepicker("setDate", date_stop);

        runsearch();
    });
});
