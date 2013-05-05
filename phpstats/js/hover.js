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


$(document).ready(function() {
    $("#search_24h").data("powertip", "Show only last 24 hours");
    $("#search_24h").powerTip({
        placement: "s",
        smartPlacement: true
    });

    $("#search_7d").data("powertip", "Show only last 7 days");
    $("#search_7d").powerTip({
        placement: "s",
        smartPlacement: true
    });

    $("#search_30d").data("powertip", "Show only last month");
    $("#search_30d").powerTip({
        placement: "s",
        smartPlacement: true
    });

    $("#search_60d").data("powertip", "Show only last 2 months");
    $("#search_60d").powerTip({
        placement: "s",
        smartPlacement: true
    });

    $("#search_year").data("powertip", "Show only last year");
    $("#search_year").powerTip({
        placement: "s",
        smartPlacement: true
    });

    $("#search_manual").data("powertip", "Show only last activities on the selected period");
    $("#search_manual").powerTip({
        placement: "s",
        smartPlacement: true
    });

    $("#search_start").data("powertip", "Start time of the query period");
    $("#search_start").powerTip({
        placement: "n",
        smartPlacement: true
    });

    $("#search_stop").data("powertip", "End time of the query period");
    $("#search_stop").powerTip({
        placement: "n",
        smartPlacement: true
    });

    $("#searchmounts_selectall").data("powertip", "Select all mountpoint");
    $("#searchmounts_selectall").powerTip({
        placement: "s",
        smartPlacement: true
    });

    $("#searchmounts_deselectall").data("powertip", "Deselect all mountpoint");
    $("#searchmounts_deselectall").powerTip({
        placement: "s",
        smartPlacement: true
    });

    $("#searchmounts_select").data("powertip", "Choose at least one mountpoint to search");
    $("#searchmounts_select").powerTip({
        placement: "s",
        smartPlacement: true
    });
});
