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
 * This section is created by Luca Cireddu <sardylan@gmail.com>
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <mysql/mysql.h>
#include <pthread.h>

#include "mysqlstats.h"
#include "cfgfile.h"
#include "logging.h"
#include "util.h"

#undef CATMODULE
#define CATMODULE "mysqlstats"

#define MYSQL_QUERY_MAXLENGTH 2048

int mysqlstats_enabled;
int mysqlstats_enabled_in_config;
MYSQL *mysql_connection;
pthread_mutex_t mysql_mutex;



/**
 * Function used to escape a string
 * @param[in] input Unescaped input string
 * @param[out] ret Escaped string
 */

char *mysqlStringEscape(const char *input)
{
    char *ret;
    char output[1024];
    size_t ln;

    if(input == NULL) {
        ret = (char *) malloc(sizeof(char));
        *(ret) = '\0';
    } else {
        mysql_real_escape_string(mysql_connection, output, input, (unsigned long) strlen(input));
        ln = strlen(output) + 1;
        ret = (char *) calloc(ln, sizeof(char));
        strcpy(ret, output);
    }

    return ret;
}



/**
 * Creates a connection-checking thread
 */

void mysqlLaunchCheckThread()
{
    pthread_t thread;
    pthread_attr_t tattr;
    int th_ret;

    mysqlstats_enabled = 0;

    ice_config_t *configuration;

    configuration = config_get_config();

    mysqlstats_enabled_in_config = (int) configuration->mysql_stats_enabled;

    config_release_config();

    if(mysqlstats_enabled_in_config == 1) {
        pthread_mutex_unlock(&mysql_mutex);

        pthread_attr_init(&tattr);
        pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);

        th_ret = pthread_create(&thread, &tattr, mysqlStatsDBConnectionCheck, (void *) NULL);

        if(th_ret)
            ERROR0("DB Connection Checking thread not created!!!");
    }
}



/**
 * Creates the global DB connection
 */

int mysqlStatsDBOpen()
{
    int ret, mysqlstats_port;
    char mysqlstats_server[128];
    char mysqlstats_user[128];
    char mysqlstats_psw[128];
    char mysqlstats_dbname[128];
    MYSQL *temp_connection;
    ice_config_t *configuration;
    unsigned int mysql_timeout;
    pthread_t thread;
    pthread_attr_t tattr;
    int th_ret;

    if(mysqlstats_enabled_in_config == 1) {
        configuration = config_get_config();

        strcpy(mysqlstats_server, configuration->mysql_stats_server);
        mysqlstats_port = (int) configuration->mysql_stats_port;
        strcpy(mysqlstats_user, configuration->mysql_stats_user);
        strcpy(mysqlstats_psw, configuration->mysql_stats_psw);
        strcpy(mysqlstats_dbname, configuration->mysql_stats_dbname);

        config_release_config();

        mysqlstats_enabled = 0;
        ret = 0;

        mysql_timeout = 2;
        temp_connection = mysql_init(NULL);
        mysql_options(temp_connection, MYSQL_OPT_CONNECT_TIMEOUT, (char *) &mysql_timeout);

        if(temp_connection == NULL) {
            ERROR2("Error creating connection: %u - %s", mysql_errno(temp_connection), mysql_error(temp_connection));
            ret = 1;
        }

        if(ret == 0) {
            INFO0("MySQL Connection data:");
            INFO1("            Server:     %s", mysqlstats_server);
            INFO1("            Port:       %d", mysqlstats_port);
            INFO1("            User:       %s", mysqlstats_user);
            INFO1("            Password:   %s", mysqlstats_psw);
            INFO1("            DB Name:    %s", mysqlstats_dbname);

            pthread_mutex_lock(&mysql_mutex);

            mysql_connection = mysql_real_connect(temp_connection, mysqlstats_server, mysqlstats_user, mysqlstats_psw, mysqlstats_dbname, mysqlstats_port, NULL, 0);

            if(mysql_connection == NULL) {
                WARN2("Error connecting to DB: %u - %s", mysql_errno(temp_connection), mysql_error(temp_connection));
                mysql_close(temp_connection);
                ret = 2;
            } else {
                mysqlstats_enabled = 1;
            }

            pthread_mutex_unlock(&mysql_mutex);
        }

        if(ret == 0)
            mysqlStatsDBCheck();
        else
            WARN0("Disabling MySQL Stats");
    }

    return ret;
}



/**
 * Destroy and close the global MySQL resource
 */

void mysqlStatsDBClose()
{
    if(mysqlstats_enabled == 1) {
        pthread_mutex_lock(&mysql_mutex);
        mysql_close(mysql_connection);
        mysqlstats_enabled = 0;
        pthread_mutex_unlock(&mysql_mutex);
    }
}



/**
 * Periodically checking the DB connection
 */

void *mysqlStatsDBConnectionCheck(void *input)
{
    int mysql_return_value;

    while(1) {
        sleep(MYSQLSTATS_DBCHECK_INTERVAL);

        mysql_return_value = 0;

        if(mysqlstats_enabled == 1) {
            pthread_mutex_lock(&mysql_mutex);
            mysql_return_value = mysql_ping(mysql_connection);
            pthread_mutex_unlock(&mysql_mutex);

            if(mysql_return_value == 0) {
                DEBUG0("MySQL ping OK");
            } else {
                ERROR2("Error %u: %s", mysql_errno(mysql_connection), mysql_error(mysql_connection));
                ERROR0("Server ping not working");
                mysqlStatsDBClose();
                mysqlStatsDBOpen();
            }
        } else {
            mysqlStatsDBOpen();
        }
    }

    pthread_exit((void *) NULL);
}



/**
 * Checking the DB and creates tables if don't exist.
 */

void mysqlStatsDBCheck()
{
    char sql_query[MYSQL_QUERY_MAXLENGTH];
    int sql_return;

    strcpy(sql_query, MYSQLSTATS_QUERY_CREATE_ONLINE);
    DEBUG1("Executing query \"%s\"", sql_query);

    sql_return = 0;

    pthread_mutex_lock(&mysql_mutex);

    if(mysqlstats_enabled == 1)
        sql_return = mysql_query(mysql_connection, sql_query);

    pthread_mutex_unlock(&mysql_mutex);

    if(sql_return != 0) {
        ERROR2("Error %u: %s", mysql_errno(mysql_connection), mysql_error(mysql_connection));
        ERROR1("SQL Query: \"%s\"", sql_query);
    }

    strcpy(sql_query, "TRUNCATE TABLE `online`");
    DEBUG1("Executing query \"%s\"", sql_query);

    sql_return = 0;

    pthread_mutex_lock(&mysql_mutex);

    if(mysqlstats_enabled == 1)
        sql_return = mysql_query(mysql_connection, sql_query);

    pthread_mutex_unlock(&mysql_mutex);

    if(sql_return != 0) {
        ERROR2("Error %u: %s", mysql_errno(mysql_connection), mysql_error(mysql_connection));
        ERROR1("SQL Query: \"%s\"", sql_query);
    }

    strcpy(sql_query, MYSQLSTATS_QUERY_CREATE_STATS);
    DEBUG1("Executing query \"%s\"", sql_query);

    sql_return = 0;

    pthread_mutex_lock(&mysql_mutex);

    if(mysqlstats_enabled == 1)
        sql_return = mysql_query(mysql_connection, sql_query);

    pthread_mutex_unlock(&mysql_mutex);

    if(sql_return != 0) {
        ERROR2("Error %u: %s", mysql_errno(mysql_connection), mysql_error(mysql_connection));
        ERROR1("SQL Query: \"%s\"", sql_query);
    }

    strcpy(sql_query, MYSQLSTATS_QUERY_CREATE_MOUNTPOINTS);
    DEBUG1("Executing query \"%s\"", sql_query);

    sql_return = 0;

    pthread_mutex_lock(&mysql_mutex);

    if(mysqlstats_enabled == 1)
        sql_return = mysql_query(mysql_connection, sql_query);

    pthread_mutex_unlock(&mysql_mutex);

    if(sql_return != 0) {
        ERROR2("Error %u: %s", mysql_errno(mysql_connection), mysql_error(mysql_connection));
        ERROR1("SQL Query: \"%s\"", sql_query);
    }
}



/**
 * Function called to add a record in DB
 * 
 * @param[in] temp_id Numerical ID of the client connection
 * @param[in] temp_start Starting Unix Timestamp of the client connection
 * @param[in] temp_ip IP Address of the client
 * @param[in] temp_mount Mount point requested
 */

void mysqlStatsConnect(unsigned long temp_id, time_t temp_start, char *temp_ip, const char *temp_agent, char *temp_mount)
{
    mysql_stats_connect_thread_data *tdata;
    size_t ln;
    int th_ret;
    pthread_t thread;
    pthread_attr_t tattr;

    if(mysqlstats_enabled == 1) {
        tdata = (mysql_stats_connect_thread_data *) malloc(sizeof(mysql_stats_connect_thread_data));

        tdata->id = temp_id;
        tdata->start = temp_start;

        ln = strlen(temp_ip);
        tdata->ip = (char *) calloc(ln+1, sizeof(char));
        strcpy(tdata->ip, temp_ip);

        if(temp_agent) {
            tdata->agent = util_url_escape(temp_agent);
        } else {
            tdata->agent = (char *) malloc(sizeof(char));
            *(tdata->agent) = '\0';
        }

        ln = strlen(temp_mount);
        tdata->mount = (char *) calloc(ln+1, sizeof(char));
        strcpy(tdata->mount, temp_mount);

        pthread_attr_init(&tattr);
        pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);

        th_ret = pthread_create(&thread, &tattr, mysqlStatsConnectThread, (void *) tdata);

        if(th_ret)
            ERROR0("Connection thread not created!!!");
    }
}



/**
 * Function called when a listener disconnect
 * 
 * @param[in] temp_id Numerical ID of the client connection
 * @param[in] temp_start Starting Unix Timestamp of the client connection
 * @param[in] temp_stop Unix Timestamp when client disconnect (not used by Icecast, always set to 0)
 */

void mysqlStatsDisconnect(unsigned long temp_id, time_t temp_start, time_t temp_stop)
{
    mysql_stats_disconnect_thread_data *tdata;
    int th_ret;
    pthread_t thread;
    pthread_attr_t tattr;

    if(mysqlstats_enabled == 1) {
        tdata = (mysql_stats_disconnect_thread_data *) malloc(sizeof(mysql_stats_disconnect_thread_data));

        tdata->id = temp_id;
        tdata->start = temp_start;
        tdata->stop = time(NULL);

        pthread_attr_init(&tattr);
        pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);

        th_ret = pthread_create(&thread, &tattr, mysqlStatsDisconnectThread, (void *) tdata);

        if(th_ret)
            ERROR0("Disconnection thread not created!!!");
    }
}



/**
 * Thread function to add a listener
 */

void *mysqlStatsConnectThread(void *input)
{
    mysql_stats_connect_thread_data *tdata;
    char sql_query[MYSQL_QUERY_MAXLENGTH];
    char *escaped_mount;
    char *escaped_ip;
    char *escaped_agent;
    MYSQL_RES *result;
    MYSQL_ROW row;
    int mysqlret, mountid;
    int sql_return;

    tdata = (mysql_stats_connect_thread_data *) input;

    INFO0("MYSQLDEBUG::: #################################################");
    INFO1("MYSQLDEBUG::: New connection, ID %lu", tdata->id);
    INFO1("MYSQLDEBUG:::     Connection timestamp: %lld", (long long) tdata->start);
    INFO1("MYSQLDEBUG:::     IP Address: %s", tdata->ip);
    INFO1("MYSQLDEBUG:::     User Agent: %s", tdata->agent);
    INFO1("MYSQLDEBUG:::     MountPoint: %s", tdata->mount);
    INFO0("MYSQLDEBUG::: #################################################");

    escaped_mount = mysqlStringEscape(tdata->mount);
    sprintf(sql_query, "SELECT COUNT(id) FROM mountpoints WHERE mount = '%s'", escaped_mount);
    free(escaped_mount);

    DEBUG1("Executing query \"%s\"", sql_query);

    mysqlret = 0;

    pthread_mutex_lock(&mysql_mutex);

    if(mysqlstats_enabled == 1) {
        mysql_query(mysql_connection, sql_query);
        result = mysql_store_result(mysql_connection);
        row = mysql_fetch_row(result);
        mysqlret = atoi(row[0]);
        mysql_free_result(result);
    }

    pthread_mutex_unlock(&mysql_mutex);

    if(mysqlret == 0) {
        escaped_mount = mysqlStringEscape(tdata->mount);
        sprintf(sql_query, "INSERT INTO mountpoints (mount) VALUES ('%s');", escaped_mount);
        free(escaped_mount);

        DEBUG1("Executing query \"%s\"", sql_query);

        sql_return = 0;

        pthread_mutex_lock(&mysql_mutex);

        if(mysqlstats_enabled == 1)
            sql_return = mysql_query(mysql_connection, sql_query);

        pthread_mutex_unlock(&mysql_mutex);

        if(sql_return != 0) {
            ERROR2("Error %u: %s", mysql_errno(mysql_connection), mysql_error(mysql_connection));
            ERROR1("SQL Query: \"%s\"", sql_query);
        }

    }

    mountid = 0;

    escaped_mount = mysqlStringEscape(tdata->mount);
    sprintf(sql_query, "SELECT id FROM mountpoints WHERE mount = '%s'", escaped_mount);
    free(escaped_mount);

    DEBUG1("Executing query \"%s\"", sql_query);

    pthread_mutex_lock(&mysql_mutex);

    if(mysqlstats_enabled == 1) {
        mysql_query(mysql_connection, sql_query);
        result = mysql_store_result(mysql_connection);
        row = mysql_fetch_row(result);
        mountid = atoi(row[0]);
        mysql_free_result(result);
    }

    pthread_mutex_unlock(&mysql_mutex);

    if(mountid != 0) {
        escaped_mount = mysqlStringEscape(tdata->mount);
        escaped_ip = mysqlStringEscape(tdata->ip);
        escaped_agent = mysqlStringEscape(tdata->agent);
        sprintf(sql_query, "INSERT INTO online (id, ip, agent, start, mount) VALUES (%lu, '%s', '%s', FROM_UNIXTIME(%lld), %d)", tdata->id, tdata->ip, tdata->agent, (long long) tdata->start, mountid);
        free(escaped_mount);
        free(escaped_ip);
        free(escaped_agent);

        DEBUG1("Executing query \"%s\"", sql_query);

        sql_return = 0;

        pthread_mutex_lock(&mysql_mutex);

        if(mysqlstats_enabled == 1)
            sql_return = mysql_query(mysql_connection, sql_query);

        pthread_mutex_unlock(&mysql_mutex);

        if(sql_return != 0) {
            ERROR2("Error %u: %s", mysql_errno(mysql_connection), mysql_error(mysql_connection));
            ERROR1("SQL Query: \"%s\"", sql_query);
        }
    }

    free(tdata->ip);
    free(tdata->agent);
    free(tdata->mount);
    free(tdata);

    pthread_exit((void *) NULL);
}



/**
 * Thread function called when a listener close the connection
 */

void *mysqlStatsDisconnectThread(void *input)
{
    mysql_stats_disconnect_thread_data *tdata;
    MYSQL_RES *result;
    MYSQL_ROW row;
    char sql_query[MYSQL_QUERY_MAXLENGTH];
    char *escaped_ip;
    char *escaped_agent;
    char ip[16];
    char agent[1025];
    int mount;
    int sql_return;
    int conn_exists;
    int rows;

    tdata = (mysql_stats_disconnect_thread_data *) input;

    INFO0("MYSQLDEBUG::: -------------------------------------------------");
    INFO1("MYSQLDEBUG::: Connection %lu Disconnected", tdata->id);
    INFO1("MYSQLDEBUG:::     Start timestamp: %lld", (long long) tdata->start);
    INFO1("MYSQLDEBUG:::     Stop timestamp: %lld", (long long) tdata->stop);
    INFO1("MYSQLDEBUG:::     Duration (seconds): %lld", (long long) (tdata->stop - tdata->start));
    INFO0("MYSQLDEBUG::: -------------------------------------------------");

    sprintf(sql_query, "SELECT ip, agent, mount FROM online WHERE id = %lld", (long long) tdata->id);

    DEBUG1("Executing query \"%s\"", sql_query);

    mount = 0;
    rows = 0;
    conn_exists = 0;

    pthread_mutex_lock(&mysql_mutex);

    if(mysqlstats_enabled == 1) {
        mysql_query(mysql_connection, sql_query);
        result = mysql_store_result(mysql_connection);

        rows = mysql_num_rows(result);

        if(rows > 0) {
            conn_exists = 1;
            row = mysql_fetch_row(result);
            strcpy(ip, row[0]);
            strcpy(agent, row[1]);
            mount = atoi(row[2]);
        }

        mysql_free_result(result);
    }

    pthread_mutex_unlock(&mysql_mutex);

    if(conn_exists > 0) {
        sprintf(sql_query, "DELETE FROM online WHERE id = %lu", tdata->id);

        DEBUG1("Executing query \"%s\"", sql_query);

        pthread_mutex_lock(&mysql_mutex);

        sql_return = 0;

        if(mysqlstats_enabled == 1)
            sql_return = mysql_query(mysql_connection, sql_query);

        pthread_mutex_unlock(&mysql_mutex);

        if(sql_return != 0) {
            ERROR2("Error %u: %s", mysql_errno(mysql_connection), mysql_error(mysql_connection));
            ERROR1("SQL Query: \"%s\"", sql_query);
        } else {
            escaped_ip = mysqlStringEscape(ip);
            escaped_agent = mysqlStringEscape(agent);
            sprintf(sql_query, "INSERT INTO stats (ip, agent, mount, start, stop, duration) VALUES ('%s', '%s', %d, FROM_UNIXTIME(%lld), FROM_UNIXTIME(%lld), %d)", ip, agent, mount, (long long) tdata->start, (long long) tdata->stop, (int) (tdata->stop - tdata->start));
            free(escaped_ip);
            free(escaped_agent);

            DEBUG1("Executing query \"%s\"", sql_query);

            sql_return = 0;

            pthread_mutex_lock(&mysql_mutex);

            if(mysqlstats_enabled == 1)
                sql_return = mysql_query(mysql_connection, sql_query);

            pthread_mutex_unlock(&mysql_mutex);

            if(sql_return != 0) {
                ERROR2("Error %u: %s", mysql_errno(mysql_connection), mysql_error(mysql_connection));
                ERROR1("SQL Query: \"%s\"", sql_query);
            }
        }
    }

    free(tdata);

    pthread_exit((void *) NULL);
}
