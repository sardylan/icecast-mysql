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
 */

#ifndef __LOGGING_H__
#define __LOGGING_H__

#include "cfgfile.h"
#include "log/log.h"

/* declare the global log descriptors */

extern int errorlog;
extern int accesslog;
extern int playlistlog;

/* these are all ERRORx and WARNx where _x_ is the number of parameters
** it takes.  it turns out most other copmilers don't have support for
** varargs macros.  that totally sucks, but this is still pretty easy.
**
** feel free to add more here if needed. 
*/

#ifdef _WIN32
#include <string.h>
#define __func__ strrchr (__FILE__, '\\') ? strrchr (__FILE__, '\\') + 1 : __FILE__
#endif

#define ERROR0(y) log_write(errorlog, 1, CATMODULE "/", __func__, y)
#define ERROR1(y, a) log_write(errorlog, 1, CATMODULE "/", __func__, y, a)
#define ERROR2(y, a, b) log_write(errorlog, 1, CATMODULE "/", __func__, y, a, b)
#define ERROR3(y, a, b, c) log_write(errorlog, 1, CATMODULE "/", __func__, y, a, b, c)
#define ERROR4(y, a, b, c, d) log_write(errorlog, 1, CATMODULE "/", __func__, y, a, b, c, d)

#define WARN0(y) log_write(errorlog, 2, CATMODULE "/", __func__, y)
#define WARN1(y, a) log_write(errorlog, 2, CATMODULE "/", __func__, y, a)
#define WARN2(y, a, b) log_write(errorlog, 2, CATMODULE "/", __func__, y, a, b)
#define WARN3(y, a, b, c) log_write(errorlog, 2, CATMODULE "/", __func__, y, a, b, c)

#define INFO0(y) log_write(errorlog, 3, CATMODULE "/", __func__, y)
#define INFO1(y, a) log_write(errorlog, 3, CATMODULE "/", __func__, y, a)
#define INFO2(y, a, b) log_write(errorlog, 3, CATMODULE "/", __func__, y, a, b)
#define INFO3(y, a, b, c) log_write(errorlog, 3, CATMODULE "/", __func__, y, a, b, c)

#define DEBUG0(y) log_write(errorlog, 4, CATMODULE "/", __func__, y)
#define DEBUG1(y, a) log_write(errorlog, 4, CATMODULE "/", __func__, y, a)
#define DEBUG2(y, a, b) log_write(errorlog, 4, CATMODULE "/", __func__, y, a, b)
#define DEBUG3(y, a, b, c) log_write(errorlog, 4, CATMODULE "/", __func__, y, a, b, c)
#define DEBUG4(y, a, b, c, d) log_write(errorlog, 4, CATMODULE "/", __func__, y, a, b, c, d)

/* CATMODULE is the category or module that logging messages come from.
** we set one here in cause someone forgets in the .c file.
*/
/*#define CATMODULE "unknown"
 */

/* this is the logging call to write entries to the access_log
** the combined log format is:
** ADDR USER AUTH DATE REQUEST CODE BYTES REFERER AGENT [TIME]
** ADDR = ip address of client
** USER = username if authenticated
** AUTH = auth type, not used, and set to "-"
** DATE = date in "[30/Apr/2001:01:25:34 -0700]" format
** REQUEST = request, ie "GET /live.ogg HTTP/1.0"
** CODE = response code, ie, 200 or 404
** BYTES = total bytes of data sent (other than headers)
** REFERER = the refering URL
** AGENT = the user agent
**
** for icecast, we add on extra field at the end, which will be 
** ignored by normal log parsers
**
** TIME = seconds that the connection lasted
** 
** this allows you to get bitrates (BYTES / TIME)
** and figure out exact times of connections
**
** it should be noted also that events are sent on client disconnect,
** so the DATE is the timestamp of disconnection.  DATE - TIME is the 
** time of connection.
*/

#define LOGGING_FORMAT_CLF "%d/%b/%Y:%H:%M:%S %z"

void logging_access(client_t *client);
void logging_playlist(const char *mount, const char *metadata, long listeners);
void restart_logging (ice_config_t *config);
void log_parse_failure (void *ctx, const char *fmt, ...);

#endif  /* __LOGGING_H__ */

