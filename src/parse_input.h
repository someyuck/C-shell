#ifndef __PARSE_INPUT_H
#define __PARSE_INPUT_H

// parse input string at prompt and return an array of structs, with the follwing struct definition:

shell_command_data_ptr create_shell_command_struct(int num_args, int fg_or_bg);
void destroy_shell_command_struct(shell_command_data_ptr spp);


// returns a list of lists of commands, each list a pipeline
pipeline *parse_input(char *input_string, int len, int *num_pipelines, int update_latest_comm_flag);

#endif