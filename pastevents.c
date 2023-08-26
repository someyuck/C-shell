#include "headers.h"

void pastevents(char **args, int num_args)
{
    if (num_args == 1)
    {
        int num_lines;
        char **lines = readlines("history.txt", &num_lines);
        if (lines == NULL) // file not created yet
        {
            return;
        }
        for (int i = 0; i < num_lines; i++)
        {
            printf("[%s]\n", lines[i]);
        }

        for (int i = 0; i < num_lines; i++)
        {
            free(lines[i]);
        }
        free(lines);
    }
    else if (num_args == 2 && strcmp(args[1], "purge") == 0)
    {
        FILE *fp = fopen("history.txt", "w");
        if (fp == NULL) // file not created yet
        {
            return;
        }
        fclose(fp);
    }
}

void store_commands()
{
    char *input_copy = (char *)malloc(sizeof(char) * (strlen(latest_prompt_input) + 1));
    strcpy(input_copy, latest_prompt_input);

    for (int i = 0; i < num_latest_commands; i++)
    {
        free(latest_commands_list[i]);
    }
    free(latest_commands_list);
    latest_commands_list = NULL;
    num_latest_commands = 0;

    int num_commands;
    shell_command_data_ptr *commands = parse_input(latest_prompt_input, strlen(latest_prompt_input), &num_commands);

    int num_lines;
    char **write_list = readlines("history.txt", &num_lines);
    if (write_list == NULL) // file not there, write will create
    {
        num_lines = 0;
    }
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
                return;
            }
            else if (commands[i]->num_args == 3 && strcmp(commands[i]->words[1], "execute") == 0)
            {
                int prev_process_idx = strtol(commands[i]->words[2], NULL, 10);
                if (prev_process_idx > num_lines)
                {
                    // fprintf(stderr, "\033[1;31mERROR: %d commands in file\033[0m\n", num_lines);
                    // command will fail so write it
                    write_string = (char *)realloc(write_string, sizeof(char) * (strlen(write_string) + 1 + strlen(latest_commands_list[i] + 3)));
                    strcat(write_string, " ");
                    strcat(write_string, latest_commands_list[i]);

                    if (commands[i]->fg_or_bg == 0)
                        strcat(write_string, " ;");
                    else
                        strcat(write_string, " &");
                }
                else
                {
                    char *prev_cmd = write_list[num_lines - prev_process_idx];
                    write_string = (char *)realloc(write_string, sizeof(char) * (strlen(write_string) + 1 + strlen(prev_cmd + 2)));
                    strcat(write_string, " ");
                    strcat(write_string, latest_commands_list[i]);
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
        char **new_list = (char **)malloc(sizeof(char *) * num_lines - 1);
        for (int i = num_lines - 14; i < num_lines; i++) // pick last 14
        {
            new_list[i - (num_lines - 14)] = write_list[i];
        }
        for (int i = 0; i < num_lines - 14; i++)
        {
            free(write_list[i]);
        }
        num_lines--;
        free(write_list);
        write_list = new_list;
    }
    write_list = (char **)realloc(write_list, sizeof(char *) * (num_lines + 1));
    write_list[num_lines++] = write_string;

    writelines("history.txt", write_list, num_lines);

    // cleanup
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
}