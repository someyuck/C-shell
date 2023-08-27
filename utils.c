#include "headers.h"

char **readlines(const char *path, int *num_lines)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
    {
        int fd = open(path, O_CREAT, 0644);
        close(fd);
        return NULL;
    }
    char buf[4096 * 20 + 1];
    char **lines = NULL;
    *num_lines = 0;

    fread(buf, sizeof(char), 4096 * 20, fp);
    fclose(fp);

    if (strcmp(buf, "") == 0) // empty file
    {
        return lines;
    }

    // printf("buf: (%s)\n", buf);
    char *line = NULL;
    while ((*num_lines) == 0 || line != NULL)
    {
        if (*num_lines == 0)
            line = strtok(buf, "\n\r");
        else
            line = strtok(NULL, "\n\r");

        if (line == NULL)
            break;

        lines = (char **)realloc(lines, sizeof(char *) * ((*num_lines) + 1));
        lines[(*num_lines)] = (char *)malloc(sizeof(char) * (strlen(line) + 1));
        strcpy(lines[(*num_lines)++], line);
    }

    // printf("readlines numlines = %d\n",*num_lines);

    return lines;
}

void writelines(const char *path, char **lines, int num_lines)
{
    FILE *fp = fopen(path, "w");
    if (fp == NULL)
    {
        int fd = open(path, O_CREAT, 0644);
        close(fd);
        FILE *fp = fopen(path, "w");
    }
    for (int i = 0; i < num_lines; i++)
    {
        fprintf(fp, "%s\n", lines[i]);
        // printf("lines [%d] : {%s}\n", i, lines[i]);
    }
    fclose(fp);
}

void execute(shell_command_data_ptr comm)
{
    if (comm != NULL)
    {
        if (strcmp(comm->words[0], "warp") == 0)
        {
            warp(comm->words, comm->num_args);
        }
        else if (strcmp(comm->words[0], "peek") == 0)
        {
            peek(comm->words, comm->num_args);
        }
        else if (strcmp(comm->words[0], "proclore") == 0)
        {
            proclore(comm->words, comm->num_args);
        }
        else if (strcmp(comm->words[0], "pastevents") == 0)
        {
            pastevents(comm->words, comm->num_args);
        }
        else
        {
            system_command(comm);
        }
    }
}