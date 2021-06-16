#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include "libft/libft.h"

typedef	struct	s_status
{
	bool	output_redirect;
	bool	input_redirect;
	bool	output_pipe;
	bool	input_pipe;
}		t_status;

void	put_error(char *error_message)
{
	ft_putstr_fd("pipex: ", 2);
	ft_putstr_fd(error_message, 2);
}

void	backup_stdio_fd(int *stdio_fd)
{
	stdio_fd[0] = dup(STDIN_FILENO);
	if (stdio_fd[0] == -1)
	{
		put_error("could not backup stdin\n");
		exit(EXIT_FAILURE);
	}
	//close(STDIN_FILENO);
	stdio_fd[1] = dup(STDOUT_FILENO);
	if (stdio_fd[1] == -1)
	{
		put_error("could not backup stdout\n");
		exit(EXIT_FAILURE);
	}
	//close(STDOUT_FILENO);
}

void	recover_stdio_fd(int *stdio_fd)
{
	if (dup2(stdio_fd[0], STDIN_FILENO) == -1)
	{
		put_error("could not recover stdin\n");
		exit(EXIT_FAILURE);
	}
	close(stdio_fd[0]);
	if (dup2(stdio_fd[1], STDOUT_FILENO) == -1)
	{
		put_error("could not recover stdout\n");
		exit(EXIT_FAILURE);
	}
	close(stdio_fd[1]);
}

void	input_redirection(char *path_name)
{
	int	fd;

	fd = open(path_name, O_RDONLY);
	if (fd == -1)
	{
		put_error("no such file or directory\n");
		exit(EXIT_FAILURE);
	}
	if (dup2(fd, STDIN_FILENO) == -1)
	{
		close(fd);
		put_error("pipex: failed to dup(2)\n");
		exit(EXIT_FAILURE);
	}
	close(fd);
}

void	output_redirection(char *path_name)
{
	int	fd;

	fd = open(path_name, O_WRONLY|O_CREAT|O_TRUNC, 0666);
	if (fd == -1)
	{
		put_error("pipex: could not open output file\n");
		exit(EXIT_FAILURE);
	}
	if (dup2(fd, STDOUT_FILENO) == -1)
	{
		close(fd);
		put_error("pipex: failed to dup(2)\n");
		exit(EXIT_FAILURE);
	}
	close(fd);
}

char	**get_paths(char **environ)
{
	char		**paths;
	int		i;

	i = 0;
	while (ft_strncmp("PATH=", environ[i], 5))
		i++;
	environ[i] += 5;
	paths = ft_split(environ[i], ':');
	return (paths);
}

void	try_exec_command(char **commands)
{
	char		path[PATH_MAX + 1];
	extern char	**environ;
	char		**paths;
	int		i;

	paths = get_paths(environ);
	i = 0;
	while (paths[i])
	{
		ft_strlcat(path, commands[0], PATH_MAX);
		free(commands[0]);
		commands[0] = path;
		execve(*commands, commands, environ);
		if (errno != ENOENT)
			break;
		i++;
	}
	//free
}

void	exec_first_command(char *command, int *pipe_fd)
{
	pid_t	pid;
	char	**commands;
	extern char	**environ;

	pid = fork();
	if (pid == 0)
	{
		close(pipe_fd[0]);
		dup2(pipe_fd[1], 1);
		commands = ft_split(command, ' ');
		execve(*commands, commands, environ);
		//try_exec_command(commands);
		close(pipe_fd[1]);
		free(commands);
	}
	wait(NULL);
}

void	exec_second_command(char *command, int *pipe_fd)
{
	pid_t	pid;
	char	**commands;
	extern char	**environ;

	pid = fork();
	if (pid == 0)
	{
		close(pipe_fd[1]);
		dup2(pipe_fd[0], 0);
		commands = ft_split(command, ' ');
		execve(*commands, commands, environ);
		//try_exec_command(commands);
		close(pipe_fd[0]);
		free(commands);
	}
}

int	main(int argc, char **argv)
{
	int	stdio_fd[2];
	int	pipe_fd[2];

	//printf("test1\n");
	pipe(pipe_fd);
	backup_stdio_fd(stdio_fd);
	input_redirection(argv[1]);
	exec_first_command(argv[2], pipe_fd);
	output_redirection(argv[4]);
	exec_second_command(argv[3], pipe_fd);
	//ft_putstr_fd("test2\n", STDOUT_FILENO);
	//printf("test2\n");
	recover_stdio_fd(stdio_fd);
	//printf("test3\n");
}
