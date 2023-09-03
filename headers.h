#ifndef HEADERS_H_
#define HEADERS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include "time.h"
#include <sys/time.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/dir.h>

#include "prompt.h"
#include "parse_input.h"
#include "warp.h"
#include "peek.h"
#include "proclore.h"
#include "system_commands.h"
#include "pastevents.h"
#include "seek.h"

#include "utils.h"

extern char *home_directory;
extern char old_pwd[4096];
extern pid_t shell_pid;

extern char *long_fg_process;
extern int long_fg_process_strlen;
extern int long_fg_process_duration;


#define MAX_BG_PROCESSES_TRACKED 15
extern pid_t bg_proc_pids[MAX_BG_PROCESSES_TRACKED];
extern char* bg_proc_names[MAX_BG_PROCESSES_TRACKED];
extern int bg_processes_count;

extern char *latest_prompt_input; // initialised in main
extern char **latest_commands_list; // initialised in parse_input()
extern int num_latest_commands; // initialised in parse_input()

#endif