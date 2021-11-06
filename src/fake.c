/*
 * Copyright (c) 2021 Luc Hondareyte
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>

#include "config.h"

FILE *lockfile = NULL;
FILE *conffile = NULL;
char config_file [256];
struct config fconfig;

void fake_quit(int r) {
	if ( r == SIGHUP ) {
		get_config(config_file, &fconfig);
		return;
	}
	fprintf (stderr, "Stopping %s ...\n", fconfig.name);
	syslog(LOG_NOTICE,"Signal received (%d), quitting.\n", r);
	fclose(lockfile);
	remove (fconfig.lock);
	closelog();
	execl("/bin/sh", "sh", "-c", (char*)fconfig.stop, NULL);
	perror("execl error");
	exit(1);
}

int main(int argc, char *argv[]) {

         // Check argument
        if ( argc <= 1 ) {
                printf("Error: You must specify a configuration file.\n");
                exit (1);
        }
	else {
		sprintf(config_file,"%s", argv[1]);
	}

         // Open configuration file passed from command line
        if ((conffile = fopen (argv[1], "r")) == NULL) {
		perror(argv[1]);
		exit(1);
        }
	else fclose(conffile);

	 // Getting valid configuration
	if (( get_config(config_file, &fconfig)) == -1 ) {
		fprintf(stderr, "%s : parse error.\n", config_file);
		exit(1);
	}
	fprintf (stderr, "Starting %s ...\n", fconfig.name);

	 // Open exclusive lock file to avoid multiple instances of daemon
	if (( lockfile  = fopen(fconfig.lock, "w")) == NULL) {
		fprintf(stderr, "Impossible de creer le verrou.\n");
		perror(fconfig.lock);
		return 1;
	}
	if (flock (fileno(lockfile), LOCK_EX | LOCK_NB) == -1) {
		fprintf(stderr, "Impossible de prendre le verrou.\n");
		perror (fconfig.lock);
		return 1;
	}

	 // Logging
	openlog(fconfig.name, LOG_PID|LOG_NDELAY, LOG_DAEMON),
		syslog(LOG_NOTICE, "Starting daemon.\n");

	 // Signals handling
	signal(SIGHUP, fake_quit);
	signal(SIGINT, fake_quit);
	signal(SIGTERM, fake_quit);

	 // Daemonize
	pid_t process_id=0;
	if ((process_id = fork()) < 0) {
		perror("fork");
		exit(1);
	}
	// Start real service and kill parent process 
	if (process_id > 0 ) {
		execl("/bin/sh", "sh", "-c", (char*)fconfig.start, NULL);
		perror("execl error");
		exit(1);
	}	

	 // Writing pid to lockfile
	setvbuf (lockfile, (char*)NULL, _IONBF, 0);
	pid_t pid=getpid();
	fprintf(lockfile, "%d", pid);
	setpriority(PRIO_PROCESS, pid, 20);

	// Sleeping Beauty in the Woods
	while(1) {
		usleep(3600000000L);
	}	
}
