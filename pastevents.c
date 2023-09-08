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
        int target_index = strtol(args[2], NULL, 10);
        if (target_index <= 0)
        {
            free(history_path);
            fprintf(stderr, "\033[1;31mpastevents : ERROR: invalid target index\033[0m\n");
            return;
        }

        if (lines == NULL) // file not created yet
        {
            free(history_path);
            fprintf(stderr, "\033[1;31mpastevents : ERROR: 0 commands in history\033[0m\n");
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

            int num_comms;
            shell_command_data_ptr *comms = parse_input(target_command, strlen(target_command), &num_comms, 0);

            for (int i = 0; i < num_comms; i++)
            {
                execute(comms[i]); // defined in utils.h
            }
        }

        for (int i = 0; i < num_lines; i++)
        {
            free(lines[i]);
        }
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

    int num_commands;
    shell_command_data_ptr *commands = parse_input(latest_prompt_input, strlen(latest_prompt_input), &num_commands, 0);
    if (commands == NULL) // command entered was empty
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

    for (int i = 0; i < num_commands; i++)
    {
        if (strcmp(commands[i]->words[0], "pastevents") == 0)
        {
            if (commands[i]->num_args == 1 || (commands[i]->num_args == 2 && strcmp(commands[i]->words[1], "purge") == 0))
            {
                // cancel writing
                for (int j = 0; j < num_commands; j++)
                    destroy_shell_command_struct(commands[j]);
                free(commands);

                if (write_list != NULL)
                {
                    for (int j = 0; j < num_lines; j++)
                        free(write_list[j]);
                    free(write_list);
                }
                free(write_string);
                free(input_copy);
                free(history_path);
                return;
            }
            else if (commands[i]->num_args == 3 && strcmp(commands[i]->words[1], "execute") == 0)
            {
                int prev_process_idx = strtol(commands[i]->words[2], NULL, 10);
                if (prev_process_idx > num_lines)
                {
                    // command will fail so don't write the input string --> assumption
                    for (int j = 0; j < num_commands; j++)
                        destroy_shell_command_struct(commands[j]);
                    free(commands);

                    if (write_list != NULL)
                    {
                        for (int j = 0; j < num_lines; j++)
                            free(write_list[j]);
                        free(write_list);
                    }
                    free(write_string);
                    free(input_copy);
                    free(history_path);
                    return;
                }
                else if (num_latest_commands >= 1)
                {
                    char *prev_cmd = write_list[num_lines - prev_process_idx];
                    write_string = (char *)realloc(write_string, sizeof(char) * (strlen(write_string) + 1 + strlen(prev_cmd) + 2));
                    strcat(write_string, " ");
                    strcat(write_string, prev_cmd);
                }
            }
        }
        else
        {
            write_string = (char *)realloc(write_string, sizeof(char) * (strlen(write_string) + 1 + strlen(latest_commands_list[i]) + 3));

            strcat(write_string, " ");
            strcat(write_string, latest_commands_list[i]);

            if (commands[i]->fg_or_bg == 0)
                strcat(write_string, " ;"); // add assumption for if the very last command is fg
            else
                strcat(write_string, " &");
        }
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
    int num_write_string_comms;
    shell_command_data_ptr *write_string_comms = parse_input(temp_write_string, strlen(temp_write_string), &num_write_string_comms, 0);

    shell_command_data_ptr *last_comms = NULL;
    int num_last_comms = 0;
    if (num_lines > 0)
    {
        char *last_cmd = (char *)malloc(sizeof(char) * (strlen(write_list[num_lines - 1]) + 1));
        strcpy(last_cmd, write_list[num_lines - 1]);
        last_comms = parse_input(last_cmd, strlen(last_cmd), &num_last_comms, 0);
    }

    if ((num_lines == 0 || (last_comms != NULL && compare_commands(last_comms, num_last_comms, write_string_comms, num_write_string_comms) != 1)) && (write_string != NULL && strcmp(write_string, "") != 0))
    {
        write_list = (char **)realloc(write_list, sizeof(char *) * (num_lines + 1));
        write_list[num_lines++] = write_string;
        // printf("write string: ''%s''\n", write_string);
    }
    else if (write_string != NULL)
        free(write_string);

    writelines(history_path, write_list, num_lines);

    // cleanup
    for (int j = 0; j < num_write_string_comms; j++)
        destroy_shell_command_struct(write_string_comms[j]);
    free(write_string_comms);
    if (last_comms != NULL)
    {
        for (int j = 0; j < num_last_comms; j++)
            destroy_shell_command_struct(last_comms[j]);
        free(last_comms);
    }
    for (int j = 0; j < num_commands; j++)
        destroy_shell_command_struct(commands[j]);
    free(commands);
    if (write_list != NULL)
    {
        for (int j = 0; j < num_lines; j++)
            free(write_list[j]);
        free(write_list);
    }
    free(input_copy);
    free(history_path);
}
