/*
 * screenfilter.c:
 *
 * Copyright (c) 2002 DecisionSoft Ltd.
 * Paul Warren (pdw) Fri Oct 25 10:21:00 2002
 *
 */

#include <sys/types.h>
#include <regex.h>
#include <stdio.h>
#include "iftop.h"
#include "options.h"

static const char rcsid[] = "$Id$";

extern options_t options ;

regex_t preg;

int screen_filter_set(char* s) {
    int r;

    if(options.screenfilter != NULL) {
        xfree(options.screenfilter);
        options.screenfilter = NULL;
        regfree(&preg);
    }

    r = regcomp(&preg, s, 0);
      
    if(r == 0) {
        options.screenfilter = s;
        return 1;
    }
    else {
        xfree(s);
        return 0;
    }
}

int screen_filter_match(char *s) {
    int r;
    if(options.screenfilter == NULL) {
        return 1;
    }

    r = regexec(&preg, s, 0, NULL, 0);
    if(r == 0) {
        return 1;
    }
    else {
        return 0;
    }
}

