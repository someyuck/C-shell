#include "headers.h"

pipeline create_pipeline(int num_commands, int fg_or_bg)
{
    pipeline p = (pipeline)malloc(sizeof(struct pipeline_struct));
    p->num_commands = num_commands;
    p->fg_or_bg = fg_or_bg;
    p->comm_list = NULL; // realloc'd in parse_input()

    return p;
}

void destroy_pipeline(pipeline p)
{
    if (p == NULL || p->comm_list == NULL)
        return;
    for (int i = 0; i < p->num_commands; i++)
        destroy_shell_command_struct(p->comm_list[i]);

    free(p);
}

int compare_pipelines(pipeline *A, int num_pipelines_A, pipeline *B, int num_pipelines_B)
{
    int ans = 1; // 0 for not equal, 1 for equal
    int i = 0;
    while (i < num_pipelines_A && i < num_pipelines_B)
    {
        int comp_pipes = 1;
        int k = 0;

        while (k < A[i]->num_commands && k < B[i]->num_commands)
        {
            int comp_structs = 1;
            int j = 0;

            while (j < A[i]->comm_list[k]->num_args && j < B[i]->comm_list[k]->num_args)
            {
                comp_structs &= (strcmp(A[i]->comm_list[k]->words[j], B[i]->comm_list[k]->words[j]) == 0);
                if (comp_structs == 0)
                    return 0;
                j++;
            }
            if ((j == A[i]->comm_list[k]->num_args && j != B[i]->comm_list[k]->num_args) || (j != A[i]->comm_list[k]->num_args && j == B[i]->comm_list[k]->num_args))
                return 0;
            else if (j == A[i]->comm_list[k]->num_args && j == B[i]->comm_list[k]->num_args)
                comp_structs &= (A[i]->comm_list[k]->fg_or_bg == B[i]->comm_list[k]->fg_or_bg);

            if (comp_structs == 0)
                return 0;
            else
                comp_pipes &= comp_structs;

            k++;
        }
        if ((k == A[i]->num_commands && k != B[i]->num_commands) || (k != A[i]->num_commands && k == B[i]->num_commands))
            return 0;
        else if (k == A[i]->num_commands && k == B[i]->num_commands)
            comp_pipes &= (A[i]->fg_or_bg == B[i]->fg_or_bg);

        if (comp_pipes == 0)
            return 0;
        else
            ans &= comp_pipes;

        i++;
    }
    if ((i == num_pipelines_A && i != num_pipelines_B) || (i != num_pipelines_A && i == num_pipelines_B))
        return 0;
    else
        return ans;
}

void handlePipeline(pipeline P)
{
    // establish pipeline redirections (stdout(i) -> stdin(i+1)), then handle redirections
    int ret;
    int pipes[P->num_commands][2]; // some weird read overflow error if (P->num_commands - 1) used instead

    int stdin_copy = dup(0);
    if (stdin_copy == -1)
    {
        fprintf(stderr, "\033[1;31mERROR: dup : errno (%d) : %s\033[0m\n", errno, strerror(errno));
        // return 0; // don't execute comm in main()
        return;
    }
    int stdout_copy = dup(1);
    if (stdout_copy == -1)
    {
        fprintf(stderr, "\033[1;31mERROR: dup : errno (%d) : %s\033[0m\n", errno, strerror(errno));
        // return 0; // don't execute comm in main()
        return;
    }

    for (int i = 0; i < P->num_commands; i++)
    {
        // create a pipe in ith array
        ret = pipe(pipes[i]);
        if (ret == -1)
        {
            fprintf(stderr, "\033[1;31mERROR: pipe : errno (%d) : %s\033[0m\n", errno, strerror(errno));
            // return 0; // don't execute comm in main()
            break;
        }

        // dup read end to stdin
        if (i != 0)
        {
            int ret_in = dup2(pipes[i - 1][0], 0);
            if (ret_in == -1)
            {
                fprintf(stderr, "\033[1;31mERROR: dup2 : errno (%d) : %s\033[0m\n", errno, strerror(errno));
                // return 0; // don't execute comm in main()
                break;
            }
        }
        // and write end to stdout
        if (i != P->num_commands - 1)
        {
            int ret_out = dup2(pipes[i][1], 1);
            if (ret_out == -1)
            {
                fprintf(stderr, "\033[1;31mERROR: dup2 : errno (%d) : %s\033[0m\n", errno, strerror(errno));
                // return 0; // don't execute comm in main()
                break;
            }
        }

        handle_redirection_and_execute(P->comm_list[i]);

        // restore std streams and close pipe
        if (i != 0)
        {
            close(pipes[i - 1][0]);
            while (dup2helper(stdin_copy, 0) == 0)
            {
            }
        }
        if (i != P->num_commands - 1)
        {
            close(pipes[i][1]);
            while (dup2helper(stdout_copy, 1) == 0)
            {
            }
        }
    }

    if (P->num_commands > 1)
        close(pipes[P->num_commands - 2][1]); // unused write end of second last pipe -- last pipe wasn't even called for pipe()

    while (dup2helper(stdin_copy, 0) == 0)
    {
    }
    close(stdin_copy);
    while (dup2helper(stdout_copy, 1) == 0)
    {
    }
    close(stdout_copy);
}