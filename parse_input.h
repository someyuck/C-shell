#ifndef __PARSE_INPUT_H
#define __PARSE_INPUT_H

// parse input string at prompt and return an array of structs, with the follwing struct definition:

struct shell_command_data
{
    int num_args; // num of elems in below array basically
    char **words;
    char fg_or_bg; // 0 for fg and 1 for bg
};

typedef struct shell_command_data *shell_command_data_ptr;

shell_command_data_ptr create_shell_command_struct(int num_args, int fg_or_bg);
void destroy_shell_command_struct(shell_command_data_ptr *spp);

shell_command_data_ptr *parse_input(char *input_string, int len);

#endif