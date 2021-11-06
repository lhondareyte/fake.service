/*
 * Copyright (c) 2018-2021 Luc Hondareyte
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
#include <string.h>
#include "config.h"

void removeDoubleCommas(char *s) {
    int j, n = strlen(s);
    for (int i = j = 0; i < n; i++)
        if (s[i] != '"')
            s[j++] = s[i];
 
    s[j] = '\0';
}

int get_config(char *filename, struct config *s) {
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

