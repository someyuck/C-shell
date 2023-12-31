#include "headers.h"

shell_command_data_ptr create_shell_command_struct(int num_args, int fg_or_bg)
{
    shell_command_data_ptr sp = (shell_command_data_ptr)malloc(sizeof(struct shell_command_data));
    sp->num_args = num_args;
    sp->fg_or_bg = fg_or_bg;
    sp->words = NULL;          // since this'll be realloc'd in the parse function
    sp->in_redir_fname = NULL; // NULL if no redirection, if there then realloc'd in the parse function
    sp->out_redir_fname = NULL;
    sp->out_concat = -1; // 0 for o/p w/ replacement, 1 for o/p with concatenation, -1 if no o/p redirection
    return sp;
}

void destroy_shell_command_struct(shell_command_data_ptr spp)
{
    if (spp == NULL || spp->words == NULL)
        return;
    free(spp->words);
    if (spp->in_redir_fname != NULL)
        free(spp->in_redir_fname);
    if (spp->out_redir_fname != NULL)
        free(spp->out_redir_fname);
    free(spp);
}

// first tokenise input string wrt ';', each token of which may or may not contain a background process (&).
// however, the last command in each token will definitely be a foreground process (either ended with ; or was the last command)
// then, we tokenise each of these tokens wrt '&', and all but the last ones will be background processes
// except if there is the bg process at the very end

// once we get hold of individual command strings, we tokenise wrt the whitespace characters (space, tab), to get each individual
// word. we store these words in the struct defined in parse_input.h
// returns an array of structs, each struct corresponding to one command

pipeline *parse_input(char *input_string, int len, int *num_pipelines, int update_latest_comm_flag)
{
    // first of all, trim the leading and trailing whitespaces.
    input_string = trim(input_string, len, &len);
    if (input_string == NULL)
        return NULL;

    pipeline *pipeline_list = NULL;

    // pipeline cur_pipeline = NULL; // realloc this array for new command structs
    // int cur_pipeline_index = 0;

    char **pipeline_semicolon_list = NULL;
    int pipeline_semicolon_list_index = 0;
    char *pipeline_token_semicolon;
    int counter_1 = 0;

    // tokenise wrt ';'
    while (counter_1 == 0 || pipeline_token_semicolon != NULL)
    {
        if (counter_1 == 0)
            pipeline_token_semicolon = strtok(input_string, ";");
        else
            pipeline_token_semicolon = strtok(NULL, ";");

        if (pipeline_token_semicolon == NULL) // no more tokens
            break;

        // add this token to tokens list
        pipeline_semicolon_list = (char **)realloc(pipeline_semicolon_list, sizeof(char *) * (pipeline_semicolon_list_index + 1));
        pipeline_semicolon_list[pipeline_semicolon_list_index++] = pipeline_token_semicolon;

        counter_1++;
    }

    int is_last_pipeline_bg = 0;
    if (pipeline_semicolon_list[pipeline_semicolon_list_index - 1][strlen(pipeline_semicolon_list[pipeline_semicolon_list_index - 1]) - 1] == '&')
        is_last_pipeline_bg = 1;
    // now loop over this list, tokenise each token wrt '&', and add all but the last command string to the list for bg processes
    // and the last one to the fg list

    char **pipeline_strings_list = NULL;
    int pipeline_strings_list_index = 0;
    int *is_pipeline_fg_or_bg = NULL; // 0 for fg, 1 for bg

    for (int i = 0; i < pipeline_semicolon_list_index; i++)
    {
        char *command_string;
        int counter_2 = 0;

        char **temp_pipeline_strings_list = NULL;
        int temp_pipeline_strings_list_index = 0;

        // add all command strings to a temp list
        while (counter_2 == 0 || command_string != NULL)
        {
            if (counter_2 == 0)
                command_string = strtok(pipeline_semicolon_list[i], "&");
            else
                command_string = strtok(NULL, "&");

            if (command_string == NULL)
                break;

            temp_pipeline_strings_list = (char **)realloc(temp_pipeline_strings_list, sizeof(char *) * (temp_pipeline_strings_list_index + 1));
            temp_pipeline_strings_list[temp_pipeline_strings_list_index++] = command_string;

            counter_2++;
        }

        // mark all but the last as bg  and the last as fg (for last i, only if it's fg)
        for (int j = 0; j < temp_pipeline_strings_list_index; j++)
        {
            pipeline_strings_list = (char **)realloc(pipeline_strings_list, sizeof(char *) * (pipeline_strings_list_index + 1));
            pipeline_strings_list[pipeline_strings_list_index] = temp_pipeline_strings_list[j];
            is_pipeline_fg_or_bg = (int *)realloc(is_pipeline_fg_or_bg, sizeof(int) * (pipeline_strings_list_index + 1));

            if ((j == temp_pipeline_strings_list_index - 1 && i != pipeline_semicolon_list_index - 1) || (j == temp_pipeline_strings_list_index - 1 && i == pipeline_semicolon_list_index - 1 && is_last_pipeline_bg == 0))
                is_pipeline_fg_or_bg[pipeline_strings_list_index++] = 0; // fg
            else
                is_pipeline_fg_or_bg[pipeline_strings_list_index++] = 1; // bg

            if (update_latest_comm_flag == 0)
                continue;
        }

        free(temp_pipeline_strings_list);
    }

    free(pipeline_semicolon_list);

    // now we have a list of pipeline strings, so make a list of (null-terminated) lists of strings, with each elementary string correspoding to an individual command in the pipeline
    // tokenize wrt '|''

    char ***pipeline_comm_str_list = NULL;
    int pipeline_comm_str_list_idx = 0;

    for (int i = 0; i < pipeline_strings_list_index; i++)
    {
        pipeline_comm_str_list = (char ***)realloc(pipeline_comm_str_list, sizeof(char **) * (pipeline_comm_str_list_idx + 1));
        pipeline_comm_str_list[pipeline_comm_str_list_idx] = NULL;

        latest_pipelines_list = (char ***)realloc(latest_pipelines_list, sizeof(char *) * (num_latest_pipelines + 1));
        latest_pipelines_list[num_latest_pipelines] = NULL;

        char *command = NULL;
        int counter_3 = 0;
        while (counter_3 == 0 || command != NULL)
        {
            if (counter_3 == 0)
                command = strtok(pipeline_strings_list[i], "|");
            else
                command = strtok(NULL, "|");

            if (command == NULL) // no more tokens
                break;

            // add this command to ith pipeline_comm_str_list
            pipeline_comm_str_list[pipeline_comm_str_list_idx] = (char **)realloc(pipeline_comm_str_list[pipeline_comm_str_list_idx], sizeof(char *) * (counter_3 + 1));
            pipeline_comm_str_list[pipeline_comm_str_list_idx][counter_3] = command;

            // add the command string to the global var latest_pipelines_list[num_latest_pipelines]
            latest_pipelines_list[num_latest_pipelines] = (char **)realloc(latest_pipelines_list[num_latest_pipelines], sizeof(char *) * (counter_3 + 1));
            latest_pipelines_list[num_latest_pipelines][counter_3] = (char *)malloc(sizeof(char) * (strlen(command) + 1));
            strcpy(latest_pipelines_list[num_latest_pipelines][counter_3], command);

            counter_3++;
        }
        pipeline_comm_str_list[pipeline_comm_str_list_idx] = (char **)realloc(pipeline_comm_str_list[pipeline_comm_str_list_idx], sizeof(char *) * (counter_3 + 1));
        pipeline_comm_str_list[pipeline_comm_str_list_idx][counter_3] = NULL; // null terminated list of strings
        pipeline_comm_str_list_idx++;

        latest_pipelines_list[num_latest_pipelines] = (char **)realloc(latest_pipelines_list[num_latest_pipelines], sizeof(char *) * (counter_3 + 1));
        latest_pipelines_list[num_latest_pipelines][counter_3] = NULL; // null terminated list of strings
        num_latest_pipelines++;
    }

    // now loop over the list, check for redirection operators, if present, add to appropriate field in the struct, and trim the string for next steps.
    // then tokenise wrt whitespaces, converting each command string into a struct, and add it to the struct list, marking fg or bg as appropriate

    for (int i = 0; i < pipeline_comm_str_list_idx; i++)
    {
        pipeline_list = (pipeline *)realloc(pipeline_list, sizeof(pipeline) * (i + 1));
        pipeline_list[i] = create_pipeline(0, is_pipeline_fg_or_bg[i]);

        while (pipeline_comm_str_list[i][pipeline_list[i]->num_commands] != NULL)
        {
            // printf("hey i: %d\n",i);
            shell_command_data_ptr sp = create_shell_command_struct(0, is_pipeline_fg_or_bg[i]);

            // trim the command string
            int comm_len;
            pipeline_comm_str_list[i][pipeline_list[i]->num_commands] = trim(pipeline_comm_str_list[i][pipeline_list[i]->num_commands], strlen(pipeline_comm_str_list[i][pipeline_list[i]->num_commands]), &comm_len);
            if (pipeline_comm_str_list[i][pipeline_list[i]->num_commands] == NULL)
            {
                break;
            }
            // if (pipeline_comm_str_list[i][pipeline_list[i]->num_commands] == NULL)
            //     printf("yessss\n");

            // check for redirection & trim the command string
            int j = 0;
            char *trimmed_comm = NULL;
            int trimmed_comm_idx = 0;
            while (j < comm_len)
            {
                // printf("j: %d numcomms: %d\n", j, pipeline_list[i]->num_commands);
                // if (pipeline_comm_str_list[i][pipeline_list[i]->num_commands] == NULL)
                //     printf("yes\n");
                // printf("i: %d num_comms: %d strlen: %ld j: %d\n", i, pipeline_list[i]->num_commands, strlen(pipeline_comm_str_list[i][pipeline_list[i]->num_commands]), j);
                if (pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] == '<') // input redirection operator
                {
                    if (sp->in_redir_fname != NULL) // just for fun implemented multiple input operators
                    {
                        free(sp->in_redir_fname);
                        sp->in_redir_fname = NULL;
                    }

                    j++;
                    while (pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] == ' ' || pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] == '\t')
                        j++;
                    if (pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] == '\0' || pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] == '>' || pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] == '<')
                    {
                        // handle error
                        fprintf(stderr, "\033[1;31mERROR: missing a file to redirect input from\033[0m\n");
                        break; // assumtion !!!
                    }
                    int fname_idx = 0;
                    while (pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] != '\0' && pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] != '>' && pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] != '<' && pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] != ' ' && pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] != '\t')
                    {
                        sp->in_redir_fname = (char *)realloc(sp->in_redir_fname, sizeof(char) * (fname_idx + 1));
                        sp->in_redir_fname[fname_idx++] = pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j++];
                    }
                    sp->in_redir_fname = (char *)realloc(sp->in_redir_fname, sizeof(char) * (fname_idx + 1));
                    sp->in_redir_fname[fname_idx] = '\0';

                    if (fname_idx > 0)
                        j--; // to allow handling of other redirection operators
                }
                else if (pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] == '>' && pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j + 1] != '>') // output w/o concatenation
                {
                    if (sp->out_redir_fname != NULL) // just for fun implemented multiple output operators
                    {
                        free(sp->out_redir_fname);
                        sp->out_redir_fname = NULL;
                    }

                    j++;
                    while (pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] == ' ' || pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] == '\t')
                        j++;
                    if (pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] == '\0' || pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] == '>' || pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] == '<')
                    {
                        // handle error
                        fprintf(stderr, "\033[1;31mERROR: missing a file to redirect output to\033[0m\n");
                        break;
                    }

                    int fname_idx = 0;
                    while (pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] != '\0' && pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] != '>' && pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] != '<' && pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] != ' ' && pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] != '\t')
                    {
                        sp->out_redir_fname = (char *)realloc(sp->out_redir_fname, sizeof(char) * (fname_idx + 1));
                        sp->out_redir_fname[fname_idx++] = pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j++];
                    }
                    sp->out_redir_fname = (char *)realloc(sp->out_redir_fname, sizeof(char) * (fname_idx + 1));
                    sp->out_redir_fname[fname_idx] = '\0';

                    sp->out_concat = 0; // no concat

                    if (fname_idx > 0)
                        j--; // to allow handling of other redirection operators
                }
                else if (pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] == '>' && pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j + 1] == '>') // output w/ concatenation
                {
                    if (sp->out_redir_fname != NULL) // just for fun implemented multiple output operators
                    {
                        free(sp->out_redir_fname);
                        sp->out_redir_fname = NULL;
                    }

                    j += 2;
                    while (pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] == ' ' || pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] == '\t')
                        j++;
                    if (pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] == '\0' || pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] == '>' || pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] == '<')
                    {
                        // handle error
                        fprintf(stderr, "\033[1;31mERROR: missing a file to redirect output to\033[0m\n");
                        break;
                    }

                    int fname_idx = 0;
                    while (pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] != '\0' && pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] != '>' && pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] != '<' && pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] != ' ' && pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j] != '\t')
                    {
                        sp->out_redir_fname = (char *)realloc(sp->out_redir_fname, sizeof(char) * (fname_idx + 1));
                        sp->out_redir_fname[fname_idx++] = pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j++];
                    }
                    sp->out_redir_fname = (char *)realloc(sp->out_redir_fname, sizeof(char) * (fname_idx + 1));
                    sp->out_redir_fname[fname_idx] = '\0';

                    sp->out_concat = 1; // concat

                    if (fname_idx > 0)
                        j--; // to allow handling of other redirection operators
                }

                else // add char to trimmed comm, only '\0' may not be added to trimmed string, so add it in the end
                {
                    trimmed_comm = (char *)realloc(trimmed_comm, sizeof(char) * (trimmed_comm_idx + 1));
                    trimmed_comm[trimmed_comm_idx++] = pipeline_comm_str_list[i][pipeline_list[i]->num_commands][j];
                }
                j++;
            }
            trimmed_comm = (char *)realloc(trimmed_comm, sizeof(char) * (trimmed_comm_idx + 1));
            trimmed_comm[trimmed_comm_idx++] = '\0';

            free(pipeline_comm_str_list[i][pipeline_list[i]->num_commands]);
            pipeline_comm_str_list[i][pipeline_list[i]->num_commands] = trimmed_comm;

            // tokenise the trimmed string wrt spaces and add the words to the struct
            int counter_3 = 0;
            char *word;
            while (counter_3 == 0 || word != NULL)
            {
                if (counter_3 == 0)
                    word = strtok(pipeline_comm_str_list[i][pipeline_list[i]->num_commands], " \t");
                else
                    word = strtok(NULL, " \t");

                if (word == NULL)
                    break;

                sp->words = (char **)realloc(sp->words, sizeof(char *) * (sp->num_args + 1));
                sp->words[(sp->num_args)++] = word;

                counter_3++;
            }

            pipeline_list[i]->comm_list = (shell_command_data_ptr *)realloc(pipeline_list[i]->comm_list, sizeof(shell_command_data_ptr) * (pipeline_list[i]->num_commands + 1));
            pipeline_list[i]->comm_list[pipeline_list[i]->num_commands] = sp;

            (pipeline_list[i]->num_commands)++;
        }
    }

    free(pipeline_strings_list);
    free(is_pipeline_fg_or_bg);

    for (int i = 0; i < pipeline_comm_str_list_idx; i++)
        free(pipeline_comm_str_list[i]);
    free(pipeline_comm_str_list);

    (*num_pipelines) = pipeline_comm_str_list_idx;

    return pipeline_list;
}
