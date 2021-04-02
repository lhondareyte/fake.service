/*
 * Copyright (c)2018-2021, Luc Hondareyte
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

#include "fake.h"

void removeDoubleCommas(char *s) {
    int j, n = strlen(s);
    for (int i = j = 0; i < n; i++)
        if (s[i] != '"')
            s[j++] = s[i];
 
    s[j] = '\0';
}

int get_config(char *filename, struct fake_config *s) {
	FILE *file = fopen (filename, "r");
	memset(s, 0, sizeof(*s));

	if (file != NULL) {
		char line[MAXBUF];
		while(fgets(line, sizeof(line), file) != NULL) {
			char *token; 	// Keywords
			char *cfline; 	// Valid configuration line
			// Skip comments
			if ( line[0] == '#' ) {
				continue;
			}
			// Skip invalid line
			if ( strchr((char *)line, '=') == 0 ) {
				continue;
			}
			else {
				cfline=(char*)line;
				removeDoubleCommas(cfline);
				token = strsep(&cfline, "=" );
				if (strcmp(token,"NAME") == 0) {
					token = strsep(&cfline, "#\r\n" );
					memcpy(s->name, token, strlen(token));
				}
				if (strcmp(token,"STOP") == 0) {
					token = strsep(&cfline, "#\r\n" );
					memcpy(s->stop, token, strlen(token));
				}
				if (strcmp(token,"START") == 0) {
					token = strsep(&cfline, "#\r\n" );
					memcpy(s->start, token, strlen(token));
				}
				if (strcmp(token,"LOCK") == 0) {
					token = strsep(&cfline, "#\r\n" );
					memcpy(s->lock, token, strlen(token));
				}
			}
		}
		fclose(file);
	} else {
		perror(filename);
		return -1;
	}
	if ((s->name[0] == '\0') || (s->stop[0] == '\0') || (s->start[0] == '\0') || (s->lock[0] == '\0')) return -1;
	return 0;
}

