#ifndef __INPUT_DEFS_H
#define __INPUT_DEFS_H

struct shell_command_data
{
    int num_args; // num of elems in below array basically
    char **words;
    int fg_or_bg; // 0 for fg and 1 for bg
    char *in_redir_fname; // fname for i/p redirection
    char *out_redir_fname; // fname for o/p redirection
    int out_concat; // whether o/p is to be concatenated or not
};

typedef struct shell_command_data *shell_command_data_ptr;

struct pipeline_struct{
    int num_commands;
    shell_command_data_ptr *comm_list;
    int fg_or_bg; // 0 for fg, 1 for bg
};

typedef struct pipeline_struct *pipeline;

#endif