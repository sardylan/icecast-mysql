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

    // Default return value
    ret = 0;

    // Getting configuration
    configuration = config_get_config();

    // Acquiring informations from config
    mysqlstats_enabled = (int) configuration->mysql_stats_enabled;
    strcpy(mysqlstats_server, configuration->mysql_stats_server);
    mysqlstats_port = (int) configuration->mysql_stats_port;
    strcpy(mysqlstats_user, configuration->mysql_stats_user);
    strcpy(mysqlstats_psw, configuration->mysql_stats_psw);
    strcpy(mysqlstats_dbname, configuration->mysql_stats_dbname);

    config_release_config();

    // Connecting to DB only if enabled in config file
    if(mysqlstats_enabled == 1) {

        // Creating temporary connection with few seconds of timeout
        mysql_timeout = 2;
        temp_connection = mysql_init(NULL);
        mysql_options(temp_connection, MYSQL_OPT_CONNECT_TIMEOUT, (char *) &mysql_timeout);

        // If error, set ret value to 1 and print debug informations
        if(temp_connection == NULL) {
            ERROR2("Error creating connection: %u - %s", mysql_errno(temp_connection), mysql_error(temp_connection));
            ret = 1;
        }

        INFO0("MySQL Connection data:");
        INFO1("            Server:     %s", mysqlstats_server);
        INFO1("            Port:       %d", mysqlstats_port);
        INFO1("            User:       %s", mysqlstats_user);
        INFO1("            Password:   %s", mysqlstats_psw);
        INFO1("            DB Name:    %s", mysqlstats_dbname);

        // Locking global MySQL resource
        pthread_mutex_lock(&mysql_mutex);

        // Trying to connect to MySQL Server
        mysql_connection = mysql_real_connect(temp_connection, mysqlstats_server, mysqlstats_user, mysqlstats_psw, mysqlstats_dbname, mysqlstats_port, NULL, 0);

        // If error, global resource became NULL and debug infos are printed
        if((ret == 0) && (mysql_connection == NULL)) {
            WARN2("Error connecting to DB: %u - %s", mysql_errno(temp_connection), mysql_error(temp_connection));
            mysql_close(temp_connection);
            ret = 2;
        }

        // Unlock global MySQL resource
        pthread_mutex_unlock(&mysql_mutex);

        // Checking DB consistency
        if(ret == 0)
            mysqlStatsDBCheck();

    }

    // On errors, disable mysql
    if(ret != 0) {
        mysqlstats_enabled = 0;
        WARN0("Disabling MySQL Stats");
    }

    // Return value
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
        pthread_mutex_unlock(&mysql_mutex);
    }
}



/**
 * Checking the DB and creates tables if don't exist.
 */

void mysqlStatsDBCheck()
{
    char sql_query[MYSQL_QUERY_MAXLENGTH];

    // If doesn't exists, creates table online
    strcpy(sql_query, "CREATE TABLE IF NOT EXISTS `online` (`id` bigint(20) unsigned NOT NULL, `ip` text(15) COLLATE utf8_bin NOT NULL, `agent` varchar(1024) COLLATE utf8_bin NOT NULL, `start` timestamp NOT NULL, `mount` bigint(20) unsigned NOT NULL, PRIMARY KEY (`id`)) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;");
    DEBUG1("Executing query \"%s\"", sql_query);

    pthread_mutex_lock(&mysql_mutex);
    if(mysql_query(mysql_connection, sql_query) != 0) {
        ERROR2("Error %u: %s", mysql_errno(mysql_connection), mysql_error(mysql_connection));
        ERROR1("SQL Query: \"%s\"", sql_query);
    }
    pthread_mutex_unlock(&mysql_mutex);

    // Truncates table online
    strcpy(sql_query, "TRUNCATE TABLE `online`");
    DEBUG1("Executing query \"%s\"", sql_query);

    pthread_mutex_lock(&mysql_mutex);
    if(mysql_query(mysql_connection, sql_query) != 0) {
        ERROR2("Error %u: %s", mysql_errno(mysql_connection), mysql_error(mysql_connection));
        ERROR1("SQL Query: \"%s\"", sql_query);
    }
    pthread_mutex_unlock(&mysql_mutex);

    // If doesn't exists, creates table stats
    strcpy(sql_query, "CREATE TABLE IF NOT EXISTS `stats` (`id` bigint(20) unsigned NOT NULL AUTO_INCREMENT, `ip` text(15) COLLATE utf8_bin NOT NULL, `agent` varchar(1024) COLLATE utf8_bin NOT NULL, `mount` varchar(128) COLLATE utf8_bin NOT NULL, `start` timestamp NOT NULL, `stop` timestamp NOT NULL, `duration` int(10) unsigned NOT NULL, PRIMARY KEY (`id`)) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin AUTO_INCREMENT=1;");
    DEBUG1("Executing query \"%s\"", sql_query);

    pthread_mutex_lock(&mysql_mutex);
    if(mysql_query(mysql_connection, sql_query) != 0) {
        ERROR2("Error %u: %s", mysql_errno(mysql_connection), mysql_error(mysql_connection));
        ERROR1("SQL Query: \"%s\"", sql_query);
    }
    pthread_mutex_unlock(&mysql_mutex);

    // If doesn't exists, creates table mountpoints
    strcpy(sql_query, "CREATE TABLE IF NOT EXISTS `mountpoints` (`id` bigint(20) unsigned NOT NULL AUTO_INCREMENT, `mount` varchar(128) COLLATE utf8_bin NOT NULL, PRIMARY KEY (`id`)) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin AUTO_INCREMENT=1;");
    DEBUG1("Executing query \"%s\"", sql_query);

    pthread_mutex_lock(&mysql_mutex);
    if(mysql_query(mysql_connection, sql_query) != 0) {
        ERROR2("Error %u: %s", mysql_errno(mysql_connection), mysql_error(mysql_connection));
        ERROR1("SQL Query: \"%s\"", sql_query);
    }
    pthread_mutex_unlock(&mysql_mutex);
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
        // Data for the thread
        tdata = (mysql_stats_connect_thread_data *) malloc(sizeof(mysql_stats_connect_thread_data));

        // Copying values to Thread structure
        tdata->id = temp_id;
        tdata->start = temp_start;

        ln = strlen(temp_ip);
        tdata->ip = (char *) calloc(ln+1, sizeof(char));
        strcpy(tdata->ip, temp_ip);

        // temp_agent may be a NULL pointer
        if(temp_agent) {
            tdata->agent = util_url_escape(temp_agent);
        } else {
            tdata->agent = (char *) malloc(sizeof(char));
            *(tdata->agent) = '\0';
        }

        ln = strlen(temp_mount);
        tdata->mount = (char *) calloc(ln+1, sizeof(char));
        strcpy(tdata->mount, temp_mount);

        // Creating detached thread
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
        // Data for the thread
        tdata = (mysql_stats_disconnect_thread_data *) malloc(sizeof(mysql_stats_disconnect_thread_data));

        // Copying data to thread structure
        tdata->id = temp_id;
        tdata->start = temp_start;
        tdata->stop = time(NULL);

        // Creating thread
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

    // Acquiring data structure
    tdata = (mysql_stats_connect_thread_data *) input;

    // Debug informations
    INFO0("MYSQLDEBUG::: #################################################");
    INFO1("MYSQLDEBUG::: New connection, ID %lu", tdata->id);
    INFO1("MYSQLDEBUG:::     Connection timestamp: %lld", (long long) tdata->start);
    INFO1("MYSQLDEBUG:::     IP Address: %s", tdata->ip);
    INFO1("MYSQLDEBUG:::     User Agent: %s", tdata->agent);
    INFO1("MYSQLDEBUG:::     MountPoint: %s", tdata->mount);
    INFO0("MYSQLDEBUG::: #################################################");

    // Checking if mountpoint already exists
    escaped_mount = mysqlStringEscape(tdata->mount);
    sprintf(sql_query, "SELECT COUNT(id) FROM mountpoints WHERE mount = '%s'", escaped_mount);
    free(escaped_mount);

    DEBUG1("Executing query \"%s\"", sql_query);

    pthread_mutex_lock(&mysql_mutex);
    mysql_query(mysql_connection, sql_query);
    result = mysql_store_result(mysql_connection);
    row = mysql_fetch_row(result);
    mysqlret = atoi(row[0]);
    mysql_free_result(result);
    pthread_mutex_unlock(&mysql_mutex);

    // If doesn't, inserts it into mountpoints table
    if(mysqlret == 0) {
        escaped_mount = mysqlStringEscape(tdata->mount);
        sprintf(sql_query, "INSERT INTO mountpoints (mount) VALUES ('%s');", escaped_mount);
        free(escaped_mount);

        DEBUG1("Executing query \"%s\"", sql_query);

        pthread_mutex_lock(&mysql_mutex);

        if(mysql_query(mysql_connection, sql_query) != 0) {
            ERROR2("Error %u: %s", mysql_errno(mysql_connection), mysql_error(mysql_connection));
            ERROR1("SQL Query: \"%s\"", sql_query);
        }

        pthread_mutex_unlock(&mysql_mutex);
    }

    // Gets numerical id of mountpoint
    escaped_mount = mysqlStringEscape(tdata->mount);
    sprintf(sql_query, "SELECT id FROM mountpoints WHERE mount = '%s'", escaped_mount);
    free(escaped_mount);

    DEBUG1("Executing query \"%s\"", sql_query);

    pthread_mutex_lock(&mysql_mutex);
    mysql_query(mysql_connection, sql_query);
    result = mysql_store_result(mysql_connection);
    row = mysql_fetch_row(result);
    mountid = atoi(row[0]);
    mysql_free_result(result);
    pthread_mutex_unlock(&mysql_mutex);

    // Adding record to DB
    escaped_mount = mysqlStringEscape(tdata->mount);
    escaped_ip = mysqlStringEscape(tdata->ip);
    escaped_agent = mysqlStringEscape(tdata->agent);
    sprintf(sql_query, "INSERT INTO online (id, ip, agent, start, mount) VALUES (%lu, '%s', '%s', FROM_UNIXTIME(%lld), %d)", tdata->id, tdata->ip, tdata->agent, (long long) tdata->start, mountid);
    free(escaped_mount);
    free(escaped_ip);
    free(escaped_agent);

    DEBUG1("Executing query \"%s\"", sql_query);

    pthread_mutex_lock(&mysql_mutex);
    if(mysql_query(mysql_connection, sql_query) != 0) {
        ERROR2("Error %u: %s", mysql_errno(mysql_connection), mysql_error(mysql_connection));
        ERROR1("SQL Query: \"%s\"", sql_query);
    }
    pthread_mutex_unlock(&mysql_mutex);

    // Free allocated memory for data structure
    // Strings are freed separately
    free(tdata->ip);
    free(tdata->agent);
    free(tdata->mount);
    free(tdata);

    // Closing thread
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
    char *escaped_mount;
    char *escaped_ip;
    char *escaped_agent;
    char ip[16];
    char agent[1025];
    int mount;

    // Acquiring data structure
    tdata = (mysql_stats_disconnect_thread_data *) input;

    // Debug informations
    INFO0("MYSQLDEBUG::: -------------------------------------------------");
    INFO1("MYSQLDEBUG::: Connection %lu Disconnected", tdata->id);
    INFO1("MYSQLDEBUG:::     Start timestamp: %lld", (long long) tdata->start);
    INFO1("MYSQLDEBUG:::     Stop timestamp: %lld", (long long) tdata->stop);
    INFO1("MYSQLDEBUG:::     Duration (seconds): %lld", (long long) (tdata->stop - tdata->start));
    INFO0("MYSQLDEBUG::: -------------------------------------------------");

    // Obtaining IP and MountPoint from "online" table
    sprintf(sql_query, "SELECT ip, agent, mount FROM online WHERE id = %lld", (long long) tdata->id);

    DEBUG1("Executing query \"%s\"", sql_query);

    pthread_mutex_lock(&mysql_mutex);
    mysql_query(mysql_connection, sql_query);
    result = mysql_store_result(mysql_connection);
    row = mysql_fetch_row(result);
    strcpy(ip, row[0]);
    strcpy(agent, row[1]);
    mount = atoi(row[2]);
    mysql_free_result(result);
    pthread_mutex_unlock(&mysql_mutex);

    // Removing record from "online" table
    sprintf(sql_query, "DELETE FROM online WHERE id = %lu", tdata->id);
    strcpy(sql_query, sql_query);
    DEBUG1("Executing query \"%s\"", sql_query);

    pthread_mutex_lock(&mysql_mutex);
    if(mysql_query(mysql_connection, sql_query) != 0) {
        ERROR2("Error %u: %s", mysql_errno(mysql_connection), mysql_error(mysql_connection));
        ERROR1("SQL Query: \"%s\"", sql_query);
    }
    pthread_mutex_unlock(&mysql_mutex);

    // Adding record to "stats" table
    escaped_mount = mysqlStringEscape(mount);
    escaped_ip = mysqlStringEscape(ip);
    escaped_agent = mysqlStringEscape(agent);
    sprintf(sql_query, "INSERT INTO stats (ip, agent, mount, start, stop, duration) VALUES ('%s', '%s', %d, FROM_UNIXTIME(%lld), FROM_UNIXTIME(%lld), %d)", ip, agent, mount, (long long) tdata->start, (long long) tdata->stop, (int) (tdata->stop - tdata->start));
    free(escaped_mount);
    free(escaped_ip);
    free(escaped_agent);

    DEBUG1("Executing query \"%s\"", sql_query);

    pthread_mutex_lock(&mysql_mutex);
    if(mysql_query(mysql_connection, sql_query) != 0) {
        ERROR2("Error %u: %s", mysql_errno(mysql_connection), mysql_error(mysql_connection));
        ERROR1("SQL Query: \"%s\"", sql_query);
    }
    pthread_mutex_unlock(&mysql_mutex);

    // Free allocated memory for data structure
    free(tdata);

    // Closing thread
    pthread_exit((void *) NULL);
}
