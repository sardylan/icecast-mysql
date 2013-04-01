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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <signal.h>

#include "thread/thread.h"
#include "avl/avl.h"
#include "httpp/httpp.h"

#include "global.h"
#include "connection.h"
#include "refbuf.h"
#include "client.h"
#include "logging.h"
#include "event.h"

#define CATMODULE "sighandler"

#ifndef _WIN32
void _sig_hup(int signo);
void _sig_die(int signo);
void _sig_ignore(int signo);
#endif

void sighandler_initialize(void)
{
#ifndef _WIN32
    signal(SIGHUP, _sig_hup);
    signal(SIGINT, _sig_die);
    signal(SIGTERM, _sig_die);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, _sig_ignore);
#endif
}

#ifndef _WIN32
void _sig_ignore(int signo)
{
    signal(signo, _sig_ignore);
}

void _sig_hup(int signo)
{
    global . schedule_config_reread = 1;
    /* some OSes require us to reattach the signal handler */
    signal(SIGHUP, _sig_hup);
}

void _sig_die(int signo)
{
    INFO1("Caught signal %d, shutting down...", signo);

    /* inform the server to start shutting down */
    global.running = ICE_HALTING;
}

#endif
