#include "headers.h"

/*BOILERPLATE BEGINS*/ //-- edited per my implementation

struct termios orig_termios;

void disableRawMode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    {
        // die("tcsetattr");
        fprintf(stderr, "\033[1;31mERROR: tcsetattr: errno(%d) : %s\033[0m\n", errno, strerror(errno));
        return;
    }
}

/**
 * Enable raw mode for the terminal
 * The ECHO feature causes each key you type to be printed to the terminal, so you can see what you’re typing.
 * Terminal attributes can be read into a termios struct by tcgetattr().
 * After modifying them, you can then apply them to the terminal using tcsetattr().
 * The TCSAFLUSH argument specifies when to apply the change: in this case, it waits for all pending output to be written to the terminal, and also discards any input that hasn’t been read.
 * The c_lflag field is for “local flags”
 */
void enableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    {
        // die("tcsetattr");
        fprintf(stderr, "\033[1;31mERROR: tcgetattr: errno(%d) : %s\033[0m\n", errno, strerror(errno));
        return;
    }
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    {
        // die("tcsetattr");
        fprintf(stderr, "\033[1;31mERROR: tcsetattr: errno(%d) : %s\033[0m\n", errno, strerror(errno));
        return;
    }
}

/*BOILERPLATE ENDS*/

void neonate(char **args, int num_args)
{
    if (num_args == 3)
    {
        char *end;
        int time_interval = strtol(args[2], &end, 10);
        if(strcmp(args[1], "-n") != 0 )
        {
            fprintf(stderr, "\033[1;31mERROR: neonate: invalid syntax\033[0m\n");
            return;
        }
        if ( errno == EINVAL || time_interval < 0 || (end == args[1] && time_interval == 0) || *end != '\0')
        {
            fprintf(stderr, "\033[1;31mERROR: neonate: invalid time interval argument\033[0m\n");
            return;
        }

        // now I fork into two processes-- the parent will constantly read input, checking for 'x'e parent receives
        // the child will print the lates created process as per time_interval
        // if the parent detects an 'x' it sends SIGKILL to the child

        int child_pid = fork();
        if (child_pid < 0)
        {
            fprintf(stderr, "\033[1;31mERROR: fork : errorno(%d) : %s\033[0m\n", errno, strerror(errno));
            return;
        }
        else if (child_pid > 0) // parent process
        {
            enableRawMode();
            char ch;

            while (read(STDIN_FILENO, &ch, 1) == 1)
            {
                if (ch == 'x')
                {
                    int ret = kill(child_pid, SIGKILL);
                    if (ret == -1)
                        fprintf(stderr, "\033[1;31mERROR: kill: errno(%d) : %s\033[0m\n", errno, strerror(errno));
                    else
                    {
                        disableRawMode();
                        return;
                    }
                }
            }
        }
        if (child_pid == 0) // child process where neonate will run
        {
            while (1)
            {
                if (time_interval > 0) // wait time_interval seconds before printing pid
                {
                    time_t prev = time(NULL);
                    while (1)
                    {
                        if ((time(NULL) - prev) >= time_interval) // > just to be safe
                            break;
                    }
                }

                // print the pid of the latest created process
                FILE *fp = fopen("/proc/loadavg", "r");
                if (fp == NULL)
                {
                    fprintf(stderr, "\033[1;31mERROR: fopen: couldn't open /proc/loadavg\033[0m\n");
                    return;
                }
                char dummy[4096];
                pid_t latest;
                fscanf(fp, "%s %s %s %s %d", dummy, dummy, dummy, dummy, &latest);
                fclose(fp);
                printf("%d\n", latest);
            }
        }
    }
    else
        fprintf(stderr, "\033[1;31mERROR: neonate: invalid syntax\033[0m\n");
}