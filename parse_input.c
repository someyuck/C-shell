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

shell_command_data_ptr *parse_input(char *input_string, int len)
{
    input_string[len - 1] = '\0'; // to skip the last '\n'

    shell_command_data_ptr *commands_list = NULL; // realloc this array for new commands
    int commands_index = 0;

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

        printf("command token wrt ; :::: #%s#\n", command_token_semicolon);

        // now tokenise this token wrt '&', and store the individual command strings in an array

        int counter_2 = 0;
        char **command_strings_list = NULL;
        char *command_string;

        while (counter_2 == 0 || command_string != NULL)
        {
            if (counter_2 == 0)
                command_string = strtok(command_token_semicolon, "&");
            else
                command_string = strtok(NULL, "&");

            if (command_string == NULL)
                break;

            printf("command string::::: #%s#\n", command_string);

            // now add this commands string to the array
            command_strings_list = (char **)realloc(command_strings_list, sizeof(char *) * (counter_2 + 1));
            // handle error if no memory !!
            assert(command_strings_list != NULL);
            command_strings_list[counter_2] = command_string;

            counter_2++;
        }

        // now that 'command_strings_list' contains each individual command string, loop through them, tokenise wrt whitespaces
        // and store the information of each command in a 'shell_command_data' struct
        for (int i = 0; i < counter_2; i++)
        {
            int counter_3 = 0;
            char *word;
            shell_command_data_ptr sp;

            if (i == counter_2 - 1)
                sp = create_shell_command_struct(0, 0); // fg process, the last command
            else
                sp = create_shell_command_struct(0, 1); // bg process

            while (counter_3 == 0 || word != NULL)
            {
                if (counter_3 == 0)
                    word = strtok(command_strings_list[i], " \t");
                else
                    word = strtok(NULL, " \t");

                if (word == NULL)
                    break;

                sp->words = (char **)realloc(sp->words, sizeof(char *) * (sp->num_args + 1));
                sp->words[++(sp->num_args)] = word;

                counter_3++;
            }

            // now add this struct to the structs list
            commands_list = (shell_command_data_ptr *)realloc(commands_list, sizeof(shell_command_data_ptr) * (commands_index + 1));
            commands_list[++commands_index] = sp;
        }

        counter_1++;
    }

    return commands_list;
}


// instead of nested tokenising do ; first then & then spaces
// tokenise wrt ;, make tokens list
// loop over tokens list, tokenise wrt &, add command strings to separate lists for fg and bg
// loop over fg and bg string lists, convert each to a struct and add it to respective struct list