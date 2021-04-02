/*
 * Copyright (c)2021, Luc Hondareyte
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
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

#include "fake.h"

FILE *lockfile = NULL;
FILE *conffile = NULL;
char config_file [256];

void fake_quit(int r) {
	if ( r == SIGHUP ) {
		get_config(config_file, &config);
		return;
	}
	fprintf (stderr, "Stopping %s ...\n", config.name);
	syslog(LOG_NOTICE,"Signal received (%d), quitting.\n", r);
	fclose(lockfile);
	remove (config.lock);
	closelog();
	execl("/bin/sh", "sh", "-c", (char*)config.stop, NULL);
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
	if (( get_config(config_file, &config)) == -1 ) {
		fprintf(stderr, "%s : parse error.\n", config_file);
		exit(1);
	}
	fprintf (stderr, "Starting %s ...\n", config.name);

	 // Open exclusive lock file to avoid multiple instances of daemon
	if (( lockfile  = fopen(config.lock, "w")) == NULL) {
		fprintf(stderr, "Impossible de creer le verrou.\n");
		perror(config.lock);
		return 1;
	}
	if (flock (fileno(lockfile), LOCK_EX | LOCK_NB) == -1) {
		fprintf(stderr, "Impossible de prendre le verrou.\n");
		perror (config.lock);
		return 1;
	}

	 // Logging
	openlog(config.name, LOG_PID|LOG_NDELAY, LOG_DAEMON),
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
		execl("/bin/sh", "sh", "-c", (char*)config.start, NULL);
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
