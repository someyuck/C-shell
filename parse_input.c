#include "headers.h"

shell_command_data_ptr create_shell_command_struct(int num_args, int fg_or_bg)
{
    shell_command_data_ptr sp = (shell_command_data_ptr)malloc(sizeof(struct shell_command_data));
    sp->num_args = num_args;
    sp->fg_or_bg = fg_or_bg;
    sp->words = NULL; // since this'll be realloc-ed in the parse function
    return sp;
}

void destroy_shell_command_struct(shell_command_data_ptr spp)
{
    if (spp == NULL || spp->words == NULL)
        return;
    free(spp->words);
    free(spp);
}

int compare_commands(shell_command_data_ptr *A, int num_comms_A, shell_command_data_ptr *B, int num_comms_B)
{
    int ans = 1; // 0 for not equal, 1 for equal
    int i = 0;
    while (i < num_comms_A && i < num_comms_B)
    {
        int comp_structs = 1;
        int j = 0;

        while (j < A[i]->num_args && j < B[i]->num_args)
        {
            comp_structs &= (strcmp(A[i]->words[j], B[i]->words[j]) == 0);
            if (comp_structs == 0)
                return 0;
            j++;
        }
        if ((j == A[i]->num_args && j != B[i]->num_args) || (j != A[i]->num_args && j == B[i]->num_args))
            return 0;
        else if (j == A[i]->num_args && j == B[i]->num_args)
            comp_structs &= (A[i]->fg_or_bg == B[i]->fg_or_bg);

        if (comp_structs == 0)
            return 0;
        else
            ans &= comp_structs;

        i++;
    }
    if ((i == num_comms_A && i != num_comms_B) || (i != num_comms_A && i == num_comms_B))
        return 0;
    else
        return ans;
}

// first tokenise input string wrt ';', each token of which may or may not contain a background process (&).
// however, the last command in each token will definitely be a foreground process (either ended with ; or was the last command)
// then, we tokenise each of these tokens wrt '&', and all but the last ones will be background processes
// except if there is the bg process at the very end

// once we get hold of individual command strings, we tokenise wrt the whitespace characters (space, tab), to get each individual
// word. we store these words in the struct defined in parse_input.h
// returns an array of structs, each struct corresponding to one command

shell_command_data_ptr *parse_input(char *input_string, int len, int *num_commands, int update_latest_comm_flag)
{
    // first of all, trim the leading and trailing whitespaces.
    input_string = trim(input_string, len, &len);
    if (input_string == NULL)
    {
        return NULL;
    }

    shell_command_data_ptr *command_structs_list = NULL; // realloc this array for new command structs
    int command_structs_list_index = 0;

    char **command_tokens_semicolon_list = NULL;
    int command_tokens_semicolon_list_index = 0;
    char *command_token_semicolon;
    int counter_1 = 0;

    // tokenise wrt ';'
    while (counter_1 == 0 || command_token_semicolon != NULL)
    {
        if (counter_1 == 0)
            command_token_semicolon = strtok(input_string, ";");
        else
            command_token_semicolon = strtok(NULL, ";");

        if (command_token_semicolon == NULL) // no more tokens
            break;

        // add this token to tokens list
        command_tokens_semicolon_list = (char **)realloc(command_tokens_semicolon_list, sizeof(char *) * (command_tokens_semicolon_list_index + 1));
        command_tokens_semicolon_list[command_tokens_semicolon_list_index++] = command_token_semicolon;

        counter_1++;
    }

    int is_last_command_bg = 0;
    if (command_tokens_semicolon_list[command_tokens_semicolon_list_index - 1][strlen(command_tokens_semicolon_list[command_tokens_semicolon_list_index - 1]) - 1] == '&')
    {
        is_last_command_bg = 1;
    }
    // now loop over this list, tokenise each token wrt '&', and add all but the last command string to the list for bg processes
    // and the last one to the fg list

    char **command_strings_list = NULL;
    int command_strings_list_index = 0;
    int *is_command_fg_or_bg = NULL; // 0 for fg, 1 for bg

    for (int i = 0; i < command_tokens_semicolon_list_index; i++)
    {
        char *command_string;
        int counter_2 = 0;

        char **temp_command_strings_list = NULL;
        int temp_command_strings_list_index = 0;

        // add all command strings to a temp list
        while (counter_2 == 0 || command_string != NULL)
        {
            if (counter_2 == 0)
                command_string = strtok(command_tokens_semicolon_list[i], "&");
            else
                command_string = strtok(NULL, "&");

            if (command_string == NULL)
                break;

            temp_command_strings_list = (char **)realloc(temp_command_strings_list, sizeof(char *) * (temp_command_strings_list_index + 1));
            temp_command_strings_list[temp_command_strings_list_index++] = command_string;

            counter_2++;
        }

        // mark all but the last as bg  and the last as fg (for last i, only if it's fg)
        for (int j = 0; j < temp_command_strings_list_index; j++)
        {
            command_strings_list = (char **)realloc(command_strings_list, sizeof(char *) * (command_strings_list_index + 1));
            command_strings_list[command_strings_list_index] = temp_command_strings_list[j];
            is_command_fg_or_bg = (int *)realloc(is_command_fg_or_bg, sizeof(int) * (command_strings_list_index + 1));

            if ((j == temp_command_strings_list_index - 1 && i != command_tokens_semicolon_list_index - 1) || (j == temp_command_strings_list_index - 1 && i == command_tokens_semicolon_list_index - 1 && is_last_command_bg == 0))
            {
                is_command_fg_or_bg[command_strings_list_index++] = 0; // fg
            }
            else
                is_command_fg_or_bg[command_strings_list_index++] = 1; // bg

            if (update_latest_comm_flag == 0)
                continue;
            // add the command string to the global var latest_commands_list
            latest_commands_list = (char **)realloc(latest_commands_list, sizeof(char *) * (num_latest_commands + 1));
            latest_commands_list[num_latest_commands] = (char *)malloc(sizeof(char) * (strlen(temp_command_strings_list[j]) + 1));
            strcpy(latest_commands_list[num_latest_commands++], temp_command_strings_list[j]);
        }

        free(temp_command_strings_list);
    }

    free(command_tokens_semicolon_list);

    // now loop over the list, tokenise wrt whitespaces, converting each command string into a struct, and add it to the struct list, marking fg or bg as appropriate
    for (int i = 0; i < command_strings_list_index; i++)
    {
        shell_command_data_ptr sp = create_shell_command_struct(0, is_command_fg_or_bg[i]);
        int counter_3 = 0;
        char *word;
        while (counter_3 == 0 || word != NULL)
        {
            if (counter_3 == 0)
                word = strtok(command_strings_list[i], " \t");
            else
                word = strtok(NULL, " \t");

            if (word == NULL)
                break;

            sp->words = (char **)realloc(sp->words, sizeof(char *) * (sp->num_args + 1));
            sp->words[(sp->num_args)++] = word;

            counter_3++;
        }

        command_structs_list = (shell_command_data_ptr *)realloc(command_structs_list, sizeof(shell_command_data_ptr) * (command_structs_list_index + 1));
        command_structs_list[command_structs_list_index++] = sp;
    }

    free(command_strings_list);
    free(is_command_fg_or_bg);

    (*num_commands) = command_structs_list_index;

    return command_structs_list;
}
