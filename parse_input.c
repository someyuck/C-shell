#include "headers.h"

shell_command_data_ptr create_shell_command_struct(int num_args, int fg_or_bg)
{
    shell_command_data_ptr sp = (shell_command_data_ptr)malloc(sizeof(struct shell_command_data));
    sp->num_args = num_args;
    sp->fg_or_bg = fg_or_bg;
    sp->words = NULL; // since this'll be realloc-ed in the parse function
    return sp;
}

void destroy_shell_command_struct(shell_command_data_ptr *spp)
{
    for (int i = 0; i < (*spp)->num_args; i++)
    {
        free((*spp)->words[i]);
    }
    free((*spp)->words);
    free((*spp));
    free(spp);
}

// first tokenise input string wrt ';', each token of which may or may not contain a background process (&).
// however, the last command in each token will definitely be a foreground process (either ended with ; or was the last command)
// then, we tokenise each of these tokens wrt '&', and all but the last ones will be background processes

// once we get hold of individual command strings, we tokenise wrt the whitespace characters (space, tab), to get each individual
// word. we store these words in the struct defined in parse_input.h
// returns an array of structs, each struct corresponding to one command

shell_command_data_ptr *parse_input(char *input_string, int len, int *num_commands)
{
    input_string[len - 1] = '\0'; // to skip the last '\n'

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

        // printf("command token wrt ; :::: #%s#\n", command_token_semicolon);

        // add this token to tokens list
        command_tokens_semicolon_list = (char **)realloc(command_tokens_semicolon_list, sizeof(char *) * (command_tokens_semicolon_list_index + 1));
        command_tokens_semicolon_list[command_tokens_semicolon_list_index++] = command_token_semicolon;

        counter_1++;
    }

    // now loop over this list, tokenise each token wrt '&', and add all but the last command string to the list for bg processes
    // and the last one to the fg list

    char **fg_command_strings_list = NULL;
    int fg_command_strings_list_index = 0;
    char **bg_command_strings_list = NULL;
    int bg_command_strings_list_index = 0;

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

        // add all but the last one to the bg list and the last one to fg list
        for (int i = 0; i < temp_command_strings_list_index; i++)
        {
            if (i == temp_command_strings_list_index - 1)
            {
                fg_command_strings_list = (char **)realloc(fg_command_strings_list, sizeof(char *) * (fg_command_strings_list_index + 1));
                fg_command_strings_list[fg_command_strings_list_index++] = temp_command_strings_list[i];
            }
            else
            {
                bg_command_strings_list = (char **)realloc(bg_command_strings_list, sizeof(char *) * (bg_command_strings_list_index + 1));
                bg_command_strings_list[bg_command_strings_list_index++] = temp_command_strings_list[i];
            }
        }

        free(temp_command_strings_list);
    }

    free(command_tokens_semicolon_list);

    // now loop over each list, tokenise wrt whitespaces, converting each command string into a struct, and add it to the struct list
    for (int i = 0; i < fg_command_strings_list_index; i++)
    {
        shell_command_data_ptr sp = create_shell_command_struct(0, 0);
        int counter_3 = 0;
        char *word;
        while (counter_3 == 0 || word != NULL)
        {
            if (counter_3 == 0)
                word = strtok(fg_command_strings_list[i], " \t");
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

    free(fg_command_strings_list);

    for (int i = 0; i < bg_command_strings_list_index; i++)
    {
        shell_command_data_ptr sp = create_shell_command_struct(0, 1);
        int counter_3 = 0;
        char *word;
        while (counter_3 == 0 || word != NULL)
        {
            if (counter_3 == 0)
                word = strtok(bg_command_strings_list[i], " \t");
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

    free(bg_command_strings_list);

    (*num_commands) = command_structs_list_index;

    for (int i = 0; i < *num_commands; i++)
    {
        if (command_structs_list[i]->fg_or_bg == 0)
            printf("fg process:: ");
        else
            printf("bg process:: ");

        for (int j = 0; j < command_structs_list[i]->num_args; j++)
        {
            printf("%s+", command_structs_list[i]->words[j]);
        }
        printf("\n");
    }

    return command_structs_list;
}

// instead of nested tokenising do ; first then & then spaces
// tokenise wrt ;, make tokens list
// loop over tokens list, tokenise wrt &, add command strings to separate lists for fg and bg
// loop over fg and bg string lists, convert each to a struct and add it to respective struct list