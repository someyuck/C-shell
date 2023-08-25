#ifndef HEADERS_H_
#define HEADERS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include "time.h"
#include <errno.h>

#include "prompt.h"
#include "parse_input.h"
#include "warp.h"
#include "peek.h"
#include "proclore.h"

extern char *home_directory;
extern char old_pwd[4096];
extern pid_t shell_pid;

#endif