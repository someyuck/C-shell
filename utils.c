#include "headers.h"

char *trim(char str[], int len, int *new_len)
{
    char *new;

    // leading whitespaces
    int i = 0;
    while (i < len)
    {
        if (str[i] != ' ' && str[i] != '\n' && str[i] != '\t' && str[i] != '\v' && str[i] != '\f' && str[i] != '\r')
            break;
        i++;
    }
    if (i == len)
    {
        return NULL;
    }

    // trailing whitespaces
    int j = len - 1;
    while (j > i)
    {
        if (str[j] != ' ' && str[j] != '\n' && str[j] != '\t' && str[j] != '\v' && str[j] != '\f' && str[j] != '\r')
            break;
        j--;
    }

    new = (char *)malloc(sizeof(char) * (j - i + 1 + 1));
    for (int x = i; x <= j; x++)
    {
        new[x - i] = str[x];
    }
    new[j - i + 1] = '\0';
    *new_len = strlen(new);

    return new;
}

char **readlines(const char *path, int *num_lines)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
    {
        // create file if not already created
        int fd = open(path, O_CREAT, 0644);
        close(fd);
        return NULL;
    }
    char line[4096];
    char **lines = NULL;
    *num_lines = 0;

    // fread(buf, sizeof(char), 4096 * 20, fp);

    while (fgets(line, sizeof(line) / sizeof(char) - 1, fp) != NULL)
    {
        // printf("%s\n", line);
        line[strlen(line) - 1] = '\0'; // omit the last \n
        lines = (char **)realloc(lines, sizeof(char *) * ((*num_lines) + 1));
        lines[(*num_lines)] = (char *)malloc(sizeof(char) * (strlen(line) + 1));
        strcpy(lines[(*num_lines)++], line);
    }

    fclose(fp);

    // // printf("buf: (%s)\n", buf);
    // char *line = NULL;
    // while ((*num_lines) == 0 || line != NULL)
    // {
    //     if (*num_lines == 0)
    //         line = strtok(buf, "\n\r");
    //     else
    //         line = strtok(NULL, "\n\r");

    //     if (line == NULL)
    //         break;

    //     lines = (char **)realloc(lines, sizeof(char *) * ((*num_lines) + 1));
    //     lines[(*num_lines)] = (char *)malloc(sizeof(char) * (strlen(line) + 1));
    //     strcpy(lines[(*num_lines)++], line);
    // }

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
        fp = fopen(path, "w");
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
            warp(comm->words, comm->num_args);
        else if (strcmp(comm->words[0], "peek") == 0)
            peek(comm->words, comm->num_args);
        else if (strcmp(comm->words[0], "proclore") == 0)
            proclore(comm->words, comm->num_args);
        else if (strcmp(comm->words[0], "pastevents") == 0)
            pastevents(comm->words, comm->num_args);
        else if (strcmp(comm->words[0], "seek") == 0)
            seek(comm->words, comm->num_args);
        else if (strcmp(comm->words[0], "activities") == 0)
            activities(comm->words, comm->num_args);
        else if (strcmp(comm->words[0], "ping") == 0)
            ping(comm->words, comm->num_args);
        else if (strcmp(comm->words[0], "bg") == 0)
            bg(comm->words, comm->num_args);
        else if (strcmp(comm->words[0], "fg") == 0)
            fg(comm->words, comm->num_args);
        else if (strcmp(comm->words[0], "neonate") == 0)
            neonate(comm->words, comm->num_args);
        else if (strcmp(comm->words[0], "iMan") == 0)
            iman(comm->words, comm->num_args);
        else
            system_command(comm);
    }
}