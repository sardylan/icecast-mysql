/* Icecast
 *
 * This program is distributed under the GNU General Public License, version 2.
 * A copy of this license is included with this source.
 *
 * Copyright 2000-2004, Jack Moffitt <jack@xiph.org, 
 *                      Michael Smith <msmith@xiph.org>,
 *                      oddsock <oddsock@xiph.org>,
 *                      Karl Heyes <karl@xiph.org>
 *                      and others (see AUTHORS for details).
 *
 * This section is created by Luca Cireddu <luca@lucacireddu.it>
 */

#ifndef __MYSQL_H
#define __MYSQL_H

#include <time.h>
#include <mysql/mysql.h>

#define MYSQLSTATS_DBCHECK_INTERVAL 10
#define MYSQLSTATS_DBCHECK_RETRY_INTERVAL 30
#define MYSQLSTATS_DBCHECK_RETRY_MAX 3

#define MYSQLSTATS_QUERY_CREATE_ONLINE "CREATE TABLE IF NOT EXISTS `online` (`id` bigint(20) unsigned NOT NULL, `ip` text(15) COLLATE utf8_bin NOT NULL, `agent` varchar(1024) COLLATE utf8_bin NOT NULL, `start` timestamp NOT NULL, `mount` bigint(20) unsigned NOT NULL, PRIMARY KEY (`id`)) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;"
#define MYSQLSTATS_QUERY_CREATE_STATS "CREATE TABLE IF NOT EXISTS `stats` (`id` bigint(20) unsigned NOT NULL AUTO_INCREMENT, `ip` text(15) COLLATE utf8_bin NOT NULL, `agent` varchar(1024) COLLATE utf8_bin NOT NULL, `mount` varchar(128) COLLATE utf8_bin NOT NULL, `start` timestamp NOT NULL, `stop` timestamp NOT NULL, `duration` int(10) unsigned NOT NULL, PRIMARY KEY (`id`)) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin AUTO_INCREMENT=1;"
#define MYSQLSTATS_QUERY_CREATE_MOUNTPOINTS "CREATE TABLE IF NOT EXISTS `mountpoints` (`id` bigint(20) unsigned NOT NULL AUTO_INCREMENT, `mount` varchar(128) COLLATE utf8_bin NOT NULL, PRIMARY KEY (`id`)) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin AUTO_INCREMENT=1;"

struct mysql_stats_connect_thread_data_t {
    unsigned long id;
    time_t start;
    char *ip;
    char *agent;
    char *mount;
};

struct mysql_stats_disconnect_thread_data_t {
    unsigned long id;
    time_t start;
    time_t stop;
};

typedef struct mysql_stats_connect_thread_data_t mysql_stats_connect_thread_data;
typedef struct mysql_stats_disconnect_thread_data_t mysql_stats_disconnect_thread_data;

char *mysqlStringEscape(const char *);

void mysqlLaunchCheckThread();

int mysqlStatsDBOpen();
void mysqlStatsDBClose();
void *mysqlStatsDBConnectionCheck(void *);

void mysqlStatsDBCheck();

void mysqlStatsConnect(unsigned long, time_t, char *, const char *, char *);
void mysqlStatsDisconnect(unsigned long, time_t, time_t);

void *mysqlStatsConnectThread(void *);
void *mysqlStatsDisconnectThread(void *);

#endif
