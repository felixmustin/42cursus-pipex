#include "main.h"

int check_file(char **argv)
{
    if ((access(argv[1], R_OK) != -1)
        /*&& (access(argv[4], W_OK) != -1)*/)
        return (1);
    else
    {
        perror("Error Description");
        return (0);
    }
}

char *check_path(char **path, char *cmd)
{
    char *cut_cmd;
    char *full_cmd;
    int j;
    
    j = 0;
    while(cmd[j] != ' ' && cmd[j] != 0)
        j++;
    cut_cmd = malloc(sizeof(char) * (j + 1));
    ft_strlcpy(cut_cmd, cmd, j+1);
    j = -1;
    while (path[++j])
    {
        full_cmd = ft_strjoin(path[j], cut_cmd); //MALLOC HERE
        if ((access(full_cmd, X_OK) != -1))
        {
            free(full_cmd);
            return (path[j]);
        }
    }
    return (NULL);
}

char *get_path(char *cmd, char **envp)
{
    char **path;
    char *good_path;
    int i;
    int j;

    i = -1;
    while(envp[++i])
    {
        if(ft_strstr(envp[i], "PATH"))
            path = ft_split((envp[i] + 5), ':'); //MALLOC HERE
    }
    good_path = check_path(path, cmd);
    if (!good_path)
        return (NULL);
    free(path);
    return(good_path);
}

void execute(char *cmd, char **envp)
{
    char **args;
    char *full_cmd;
    char *path;

    path = get_path(cmd, envp);
    if (!path)
        return(perror("command"));
    full_cmd = ft_strjoin(path, cmd);
    args = ft_split(full_cmd, ' '); 
    if (!args)
        return(perror("malloc"));
    //free(path);
    //free(full_cmd);
    execve(args[0], args, envp);
    perror("execve");
    //free(args);
}

void cmd1_process(int f1, int pipes[2], char **argv, char **envp)
{
    char *cmd;
    char **args;
    // dup2 close stdin, f1 becomes the new stdin
    dup2(f1, STDIN_FILENO); // we want f1 to be execve() input
    dup2(pipes[1], STDOUT_FILENO); // we want end[1] to be execve() stdout
    close(pipes[0]); //always close the end of the pipe you don't use, as long as the pipe is open, the other end will be waiting for some kind of input and will not be able to finish its process
    close(f1);
    execute(argv[2], envp);
}


void cmd2_process(int f2, int pipe[2], char **argv, char **envp)
{
    int status;

    waitpid(-1, &status, 0);
    dup2(f2, STDOUT_FILENO);
    dup2(pipe[0], STDIN_FILENO);
    close(pipe[1]);
    close(f2);
    execute(argv[3], envp);
}

void pipex(int f1, int f2, char **argv, char **envp)
{
    int status;
    int pipes[2];
    pid_t cmd1;
    pid_t cmd2;

    if(pipe(pipes) == -1)
        return (perror("pipe"));
    cmd1 = fork();
    if (cmd1 == -1)
        return (perror("fork"));
    if (!cmd1)
        cmd1_process(f1, pipes, argv, envp);
    cmd2 = fork();
    if (cmd2 == -1)
        return (perror("fork"));
    if (!cmd2)
        cmd2_process(f2, pipes, argv, envp);
    close(pipes[0]);
    close(pipes[1]);
    if (waitpid(cmd1, &status, 0) == -1)
        return (perror("waitpid"));
    if (waitpid(cmd2, &status, 0) == -1)
        return (perror("waitpid"));
}

int main(int argc, char **argv, char **envp)
{
    int f1;
    int f2;

    if (argc == 5)
    {
        if (!check_file(argv))
            return (0);
        f1 = open(argv[1], O_RDONLY);
        f2 = open(argv[4], O_CREAT | O_RDWR | O_TRUNC);
        if (f1 < 0 || f2 < 0)
            return (0);
        pipex(f1, f2, argv, envp);
        return (0);
    }
    else
    {
        printf("Wrong number of args");
        return (0);
    }
}