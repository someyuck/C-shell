#include "headers.h"

char *home_directory;
char old_pwd[4096];
pid_t shell_pid;
char *long_fg_process;
int long_fg_process_strlen;
int long_fg_process_duration;
pid_t bg_proc_pids[MAX_BG_PROCESSES_TRACKED];
char *bg_proc_names[MAX_BG_PROCESSES_TRACKED];
int bg_processes_count;
char *latest_prompt_input;
char ***latest_pipelines_list;
int num_latest_pipelines;
pid_t cur_fg_child_pid;
char *cur_fg_child_pname;

int main()
{
    // Keep accepting commands

    // set signal handlers
    assign_signal_handlers();

    home_directory = getcwd(NULL, 0);
    old_pwd[0] = '\0';
    shell_pid = getpid();
    bg_processes_count = 0;
    for (int i = 0; i < MAX_BG_PROCESSES_TRACKED; i++)
    {
        bg_proc_pids[i] = -1;
        bg_proc_names[i] = NULL;
    }
    latest_prompt_input = NULL;
    latest_pipelines_list = NULL;
    num_latest_pipelines = 0;

    cur_fg_child_pid = -1;
    cur_fg_child_pname = NULL;

    while (1)
    {
        // Print appropriate prompt with username, systemname and directory before accepting input

        prompt();
        char input[4096];
        fgets(input, 4096, stdin);

        if (feof(stdin) != 0)
        {
            handle_CTRL_D();
            free(home_directory);
            if (latest_prompt_input != NULL)
                free(latest_prompt_input);
            if (cur_fg_child_pname != NULL)
                free(cur_fg_child_pname);
            if (long_fg_process != NULL)
                free(long_fg_process);

            for (int i = 0; i < num_latest_pipelines; i++)
            {
                int j = 0;
                while (latest_pipelines_list[i][j] != NULL)
                    free(latest_pipelines_list[i][j++]);
                free(latest_pipelines_list[i]);
            }
            return 0;
        }

        latest_prompt_input = (char *)malloc(sizeof(char) * (strlen(input) + 1));
        strcpy(latest_prompt_input, input);

        // to handle bg processes that exited/terminated:
        for (int j = 0; j < MAX_BG_PROCESSES_TRACKED; j++)
        {
            if (bg_processes_count == 0) // no children left to trace
                break;
            handle_bg_process_exits();
        }

        // free up space in global list of latest command strings
        if (latest_pipelines_list != NULL)
        {
            for (int i = 0; i < num_latest_pipelines; i++)
            {
                int j = 0;
                while (latest_pipelines_list[i][j] != NULL)
                    free(latest_pipelines_list[i][j++]);
                free(latest_pipelines_list[i]);
            }
            free(latest_pipelines_list);
            latest_pipelines_list = NULL;
            num_latest_pipelines = 0;
        }

        int num_pipelines;
        pipeline *pipelines = parse_input(input, strlen(input), &num_pipelines, 1);

        if (pipelines != NULL)
        {
            for (int i = 0; i < num_pipelines; i++)
            {
                // establish redirections, if any, overwriting pipeline redirections
                handlePipeline(pipelines[i]);
            }
        }
        store_commands();
    }

    free(home_directory);
}
