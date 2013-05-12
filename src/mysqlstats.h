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

int mysqlStatsDBOpen();
void mysqlStatsDBClose();

void mysqlStatsDBCheck();

void mysqlStatsConnect(unsigned long, time_t, char *, const char *, char *);
void mysqlStatsDisconnect(unsigned long, time_t, time_t);

void *mysqlStatsConnectThread(void *);
void *mysqlStatsDisconnectThread(void *);

#endif
