#include "headers.h"

void pastevents(char **args, int num_args)
{
    char *history_path = (char *)malloc(sizeof(char) * (strlen(home_directory) + strlen("/history.txt") + 1));
    strcpy(history_path, home_directory);
    strcat(history_path, "/history.txt");
    if (num_args == 1)
    {
        int num_lines;
        char **lines = readlines(history_path, &num_lines);
        if (lines == NULL) // file not created yet
        {
            int fd = open(history_path, O_CREAT, 0644);
            close(fd);
            free(history_path);
            return;
        }
        for (int i = 0; i < num_lines; i++)
        {
            printf("%s\n", lines[i]);
        }

        for (int i = 0; i < num_lines; i++)
        {
            free(lines[i]);
        }
        free(lines);
        free(history_path);
    }
    else if (num_args == 2 && strcmp(args[1], "purge") == 0)
    {
        FILE *fp = fopen(history_path, "w");
        if (fp == NULL) // file not created yet
        {
            int fd = open(history_path, O_CREAT, 0644);
            close(fd);
            fp = fopen(history_path, "w");
        }
        fclose(fp);
        free(history_path);
    }
    else if (num_args == 3 && strcmp(args[1], "execute") == 0)
    {
        int num_lines;
        char **lines = readlines(history_path, &num_lines);
        char *end;
        int target_index = strtol(args[2], &end, 10);
        if (errno == EINVAL || target_index <= 0 || *end != '\0')
        {
            free(history_path);
            fprintf(stderr, "\033[1;31mpastevents : ERROR: invalid target index\033[0m\n");
            for (int i = 0; i < num_lines; i++)
                free(lines[i]);
            return;
        }

        if (lines == NULL) // file not created yet
        {
            free(history_path);
            fprintf(stderr, "\033[1;31mpastevents : ERROR: 0 commands in history\033[0m\n");
            for (int i = 0; i < num_lines; i++)
                free(lines[i]);
            return;
        }

        if (target_index > num_lines)
        {
            fprintf(stderr, "\033[1;31mpastevents : ERROR: %d commands in history\033[0m\n", num_lines);
            for (int i = 0; i < num_lines; i++)
                free(lines[i]);
            free(lines);
            free(history_path);
            return;
        }
        else
        {
            char *target_command = lines[num_lines - target_index];

            int num_pipelines;
            pipeline *pipelines = parse_input(target_command, strlen(target_command), &num_pipelines, 0);

            for (int i = 0; i < num_pipelines; i++)
                handlePipeline(pipelines[i]); // defined in pipeline.h
        }

        for (int i = 0; i < num_lines; i++)
            free(lines[i]);
        free(lines);
        free(history_path);
    }
}

void store_commands()
{
    char *history_path = (char *)malloc(sizeof(char) * (strlen(home_directory) + strlen("/history.txt") + 1));
    strcpy(history_path, home_directory);
    strcat(history_path, "/history.txt");

    char *input_copy = (char *)malloc(sizeof(char) * (strlen(latest_prompt_input) + 1));
    strcpy(input_copy, latest_prompt_input);

    int num_pipelines;
    pipeline *pipelines = parse_input(latest_prompt_input, strlen(latest_prompt_input), &num_pipelines, 0);
    if (pipelines == NULL) // command entered was empty
    {
        free(history_path);
        free(input_copy);
        return;
    }

    int num_lines;
    char **write_list = readlines(history_path, &num_lines);
    if (write_list == NULL) // file empty
        num_lines = 0;

    char *write_string = (char *)malloc(sizeof(char) * 1);
    strcpy(write_string, "");

    for (int i = 0; i < num_pipelines; i++)
    {
        shell_command_data_ptr *commands = pipelines[i]->comm_list;
        for (int j = 0; j < pipelines[i]->num_commands; j++)
        {
            int l;
            char *temp;
            temp = trim(latest_pipelines_list[i][j], strlen(latest_pipelines_list[i][j]), &l);
            free(latest_pipelines_list[i][j]);
            latest_pipelines_list[i][j] = temp;
            if (strcmp(commands[j]->words[0], "pastevents") == 0)
            {
                if (commands[j]->num_args == 1 || (commands[j]->num_args == 2 && strcmp(commands[j]->words[1], "purge") == 0))
                {
                    // cancel writing
                    for (int x = 0; x < num_pipelines; x++)
                        destroy_pipeline(pipelines[x]);
                    free(pipelines);

                    if (write_list != NULL)
                    {
                        for (int x = 0; x < num_lines; x++)
                            free(write_list[x]);
                        free(write_list);
                    }
                    free(write_string);
                    free(input_copy);
                    free(history_path);
                    return;
                }
                else if (commands[j]->num_args == 3 && strcmp(commands[j]->words[1], "execute") == 0)
                {
                    int prev_process_idx = strtol(commands[j]->words[2], NULL, 10); // a failed pastevents execute (cuz of the index) wouldn't have been stored so no need to check
                    if (prev_process_idx > num_lines || prev_process_idx <= 0)
                    {
                        // command will fail so don't write the input string --> assumption
                        for (int x = 0; x < num_pipelines; x++)
                            destroy_pipeline(pipelines[x]);
                        free(pipelines);

                        if (write_list != NULL)
                        {
                            for (int x = 0; x < num_lines; x++)
                                free(write_list[x]);
                            free(write_list);
                        }
                        free(write_string);
                        free(input_copy);
                        free(history_path);
                        return;
                    }
                    else
                    {
                        int len;
                        char *prev_cmd = trim(write_list[num_lines - prev_process_idx], strlen(write_list[num_lines - prev_process_idx]), &len);
                        if (prev_cmd[len - 1] == ';') // as we will be adding a ';', if the current last command in the input string
                            prev_cmd[len - 1] = '\0';
                        printf("prev: [%s]\n", prev_cmd);
                        write_string = (char *)realloc(write_string, sizeof(char) * (strlen(write_string) + 1 + strlen(prev_cmd) + 2));
                        strcat(write_string, " ");
                        strcat(write_string, prev_cmd);
                        printf("ws: [%s]\n", write_string);
                    }
                }
            }
            else
            {
                write_string = (char *)realloc(write_string, sizeof(char) * (strlen(write_string) + 1 + strlen(latest_pipelines_list[i][j]) + 1));
                strcat(write_string, " ");
                strcat(write_string, latest_pipelines_list[i][j]);
            }

            // add   '|' if current command is not the last one
            if (j != pipelines[i]->num_commands - 1)
            {
                write_string = (char *)realloc(write_string, sizeof(char) * (strlen(write_string) + 2 + 1));
                strcat(write_string, " |");
            }
        }

        // since the whole pipeline will be either fg or bg (know that bg won't be given, still just storing, not handled in execution)
        int l;
        char *temp;
        temp = trim(write_string, strlen(write_string), &l);
        free(write_string);
        write_string = temp;

        write_string = (char *)realloc(write_string, sizeof(char) * (strlen(write_string) + 3));
        if (pipelines[i]->fg_or_bg == 0)
            strcat(write_string, " ;"); // add assumption for if the very last command is fg
        else
            strcat(write_string, " &"); // not gonna be tested but still
    }

    if (num_lines >= 15) // have to handle > as life is dogshit
    {
        char **new_list = (char **)malloc(sizeof(char *) * 14);
        for (int i = num_lines - 14; i < num_lines; i++) // pick last 14
        {
            new_list[i - (num_lines - 14)] = (char *)malloc(sizeof(char) * (strlen(write_list[i]) + 1));
            strcpy(new_list[i - (num_lines - 14)], write_list[i]);
        }
        for (int i = 0; i < num_lines; i++)
        {
            free(write_list[i]);
        }
        num_lines = 14;
        free(write_list);
        write_list = new_list;
    }

    int temp;
    write_string = trim(write_string, strlen(write_string), &temp);

    // compare write string with last stored commands, only add if not the same commands (can't use string comparison here due to my extra processing)
    char *temp_write_string = (char *)malloc(sizeof(char) * (strlen(write_string) + 1));
    strcpy(temp_write_string, write_string);
    int num_write_string_pipelines;
    pipeline *write_string_pipelines = parse_input(temp_write_string, strlen(temp_write_string), &num_write_string_pipelines, 0);

    pipeline *last_pipelines = NULL;
    int num_last_pipelines = 0;
    if (num_lines > 0)
    {
        char *last_cmd = (char *)malloc(sizeof(char) * (strlen(write_list[num_lines - 1]) + 1));
        strcpy(last_cmd, write_list[num_lines - 1]);
        last_pipelines = parse_input(last_cmd, strlen(last_cmd), &num_last_pipelines, 0);
    }

    if ((num_lines == 0 || (last_pipelines != NULL && compare_pipelines(last_pipelines, num_last_pipelines, write_string_pipelines, num_write_string_pipelines) != 1)) && (write_string != NULL && strcmp(write_string, "") != 0))
    {
        write_list = (char **)realloc(write_list, sizeof(char *) * (num_lines + 1));
        write_list[num_lines++] = write_string;
        // printf("write string: ''%s''\n", write_string);
    }
    else if (write_string != NULL)
        free(write_string);

    writelines(history_path, write_list, num_lines);

    // cleanup
    for (int x = 0; x < num_write_string_pipelines; x++)
        destroy_pipeline(write_string_pipelines[x]);
    free(write_string_pipelines);

    if (last_pipelines != NULL)
    {
        for (int x = 0; x < num_last_pipelines; x++)
            destroy_pipeline(last_pipelines[x]);
        free(last_pipelines);
    }
    for (int x = 0; x < num_pipelines; x++)
        destroy_pipeline(pipelines[x]);
    free(pipelines);
    if (write_list != NULL)
    {
        for (int j = 0; j < num_lines; j++)
            free(write_list[j]);
        free(write_list);
    }
    free(input_copy);
    free(history_path);
}
