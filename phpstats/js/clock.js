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
    setInterval(function() {
        var current_time = new Date();

        var date_year = current_time.getFullYear();
        var date_month = ((current_time.getMonth() + 1) < 10) ? "0" + (current_time.getMonth() + 1) : (current_time.getMonth() + 1);
        var date_day = ((current_time.getDate() ) < 10) ? "0" + (current_time.getDate() ) : (current_time.getDate() );

        var date_hour = ((current_time.getHours() ) < 10) ? "0" + (current_time.getHours() ) : (current_time.getHours() );
        var date_minutes = ((current_time.getMinutes() ) < 10) ? "0" + (current_time.getMinutes() ) : (current_time.getMinutes() );
        var date_seconds = ((current_time.getSeconds() ) < 10) ? "0" + (current_time.getSeconds() ) : (current_time.getSeconds() );

        var output_date = date_year + "-" + date_month + "-" + date_day + " " + date_hour + ":" + date_minutes + ":" + date_seconds;

        $("#value_clock").html(output_date);
    }, 1000);
});
