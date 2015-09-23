#include "test.h"
#include <stdarg.h>
#include <sys/types.h>
#include <sys/wait.h>

static void writeln(int fd, const char *s);
static void readln_ok(int fd, const char *expect);

static pid_t fork_zpush(int fd, ...);
static pid_t fork_zpull(int fd, ...);

TESTS {
	alarm(5);

	subtest { /* missing --bind / --connect option to zpush */
		int pfd[2];
		int rc;

		rc = pipe(pfd);
		if (rc != 0)
			BAIL_OUT("pipe(pfd[]) failed!");

		pid_t pid = fork();
		if (pid < 0)
			BAIL_OUT("fork() failed");

		if (pid == 0) {
			dup2(pfd[1], 1);
			dup2(pfd[1], 2);
			close(pfd[0]);
			close(pfd[1]);

			char *argv[] = {"zpush", NULL};
			exit(ztk_push(1, argv));
		}

		close(pfd[1]);
		readln_ok(pfd[0], "zpush: you must specify either --bind or --connect option(s)");
		close(pfd[0]);

		int st;
		waitpid(pid, &st, 0);
		is_int(WEXITSTATUS(st), 1, "zpush exits with rc 1 when args are incorrect");
	}

	subtest { /* both --bind / --connect options to zpush */
		int pfd[2];
		int rc;

		rc = pipe(pfd);
		if (rc != 0)
			BAIL_OUT("pipe(pfd[]) failed!");

		pid_t pid = fork();
		if (pid < 0)
			BAIL_OUT("fork() failed");

		if (pid == 0) {
			dup2(pfd[1], 1);
			dup2(pfd[1], 2);
			close(pfd[0]);
			close(pfd[1]);

			char *argv[] = {"zpush", "--bind", "tcp://here:1234", "--connect", "tcp://there:2345", NULL};
			exit(ztk_push(5, argv));
		}

		close(pfd[1]);
		readln_ok(pfd[0], "zpush: you must specify either --bind or --connect option(s)");
		close(pfd[0]);

		int st;
		waitpid(pid, &st, 0);
		is_int(WEXITSTATUS(st), 1, "zpush exits with rc 1 when args are incorrect");
	}

	subtest { /* missing --bind / --connect option to zpull */
		int pfd[2];
		int rc;

		rc = pipe(pfd);
		if (rc != 0)
			BAIL_OUT("pipe(pfd[]) failed!");

		pid_t pid = fork();
		if (pid < 0)
			BAIL_OUT("fork() failed");

		if (pid == 0) {
			dup2(pfd[1], 1);
			dup2(pfd[1], 2);
			close(pfd[0]);
			close(pfd[1]);

			char *argv[] = {"zpull", NULL};
			exit(ztk_pull(1, argv));
		}

		close(pfd[1]);
		readln_ok(pfd[0], "zpull: you must specify either --bind or --connect option(s)");
		close(pfd[0]);

		int st;
		waitpid(pid, &st, 0);
		is_int(WEXITSTATUS(st), 1, "zpull exits with rc 1 when args are incorrect");
	}

	subtest { /* both --bind / --connect options to zpull */
		int pfd[2];
		int rc;

		rc = pipe(pfd);
		if (rc != 0)
			BAIL_OUT("pipe(pfd[]) failed!");

		pid_t pid = fork();
		if (pid < 0)
			BAIL_OUT("fork() failed");

		if (pid == 0) {
			dup2(pfd[1], 1);
			dup2(pfd[1], 2);
			close(pfd[0]);
			close(pfd[1]);

			char *argv[] = {"zpull", "--bind", "tcp://here:1234", "--connect", "tcp://there:2345", NULL};
			exit(ztk_pull(5, argv));
		}

		close(pfd[1]);
		readln_ok(pfd[0], "zpull: you must specify either --bind or --connect option(s)");
		close(pfd[0]);

		int st;
		waitpid(pid, &st, 0);
		is_int(WEXITSTATUS(st), 1, "zpull exits with rc 1 when args are incorrect");
	}

	subtest { /* BIND pull, CONNECT push */
		int pipe_in[2], pipe_out[2];
		int rc;

		rc = pipe(pipe_in);
		if (rc != 0)
			BAIL_OUT("pipe(pipe_in[]) failed!");

		rc = pipe(pipe_out);
		if (rc != 0)
			BAIL_OUT("pipe(pipe_out[]) failed!");

		/* (test) | zpush -> zpull | (test) */
		pid_t zpush_pid = fork_zpush(pipe_in[0], "zpush", "--connect", "tcp://127.0.0.1:32909", NULL);
		pid_t zpull_pid = fork_zpull(pipe_out[1], "zpull", "--bind", "tcp://127.0.0.1:32909", NULL);

		/* write some stuff to the pipe */
		close(pipe_in[0]);
		writeln(pipe_in[1], "BEACON|12345");
		writeln(pipe_in[1], "RETRANS|<whatever>|blah");
		writeln(pipe_in[1], "PING");
		close(pipe_in[1]);

		/* read some stuff out the other end of the pipe */
		close(pipe_out[1]);
		readln_ok(pipe_out[0], "BEACON|12345");
		readln_ok(pipe_out[0], "RETRANS|<whatever>|blah");
		readln_ok(pipe_out[0], "PING");
		close(pipe_out[0]);

		int st;
		waitpid(zpush_pid, &st, 0);
		is_int(st, 0, "zpush agent should exit normally");

		kill(zpull_pid, SIGTERM);
		waitpid(zpull_pid, &st, 0);
		is_int(st, 0, "zpull agent should exit normally");
	}

	subtest { /* BIND push, CONNECT pull */
		int pipe_in[2], pipe_out[2];
		int rc;

		rc = pipe(pipe_in);
		if (rc != 0)
			BAIL_OUT("pipe(pipe_in[]) failed!");

		rc = pipe(pipe_out);
		if (rc != 0)
			BAIL_OUT("pipe(pipe_out[]) failed!");

		pid_t zpush_pid = fork_zpush(pipe_in[0], "zpush", "--bind", "tcp://127.0.0.1:32909", NULL);
		pid_t zpull_pid = fork_zpull(pipe_out[1], "zpull", "--connect", "tcp://127.0.0.1:32909", NULL);

		/* write some stuff to the pipe */
		close(pipe_in[0]);
		writeln(pipe_in[1], "BEACON|12345");
		writeln(pipe_in[1], "RETRANS|<whatever>|blah");
		writeln(pipe_in[1], "PING");
		close(pipe_in[1]);

		/* read some stuff out the other end of the pipe */
		close(pipe_out[1]);
		readln_ok(pipe_out[0], "BEACON|12345");
		readln_ok(pipe_out[0], "RETRANS|<whatever>|blah");
		readln_ok(pipe_out[0], "PING");
		close(pipe_out[0]);

		int st;
		waitpid(zpush_pid, &st, 0);
		is_int(st, 0, "zpush agent should exit normally");

		kill(zpull_pid, SIGTERM);
		waitpid(zpull_pid, &st, 0);
		is_int(st, 0, "zpull agent should exit normally");
	}
}

static void writeln(int fd, const char *s)
{
	write(fd, s, strlen(s));
	write(fd, "\n", 1);
}

static void readln_ok(int fd, const char *expect)
{
	size_t len = strlen(expect) + 1;
	char *buf = vcalloc(len + 1, sizeof(char));
	size_t n = read(fd, buf, len);
	if (n > 0 && buf[n - 1] == '\n')
		buf[n - 1] = '\0';
	is(buf, expect, "zpull output %s", expect);
	free(buf);
}

static char** vargv(va_list vl, int *argc)
{
	va_list copy;
	char *s;

	*argc = 0;

	va_copy(copy, vl);
	while (va_arg(copy, void*))
		(*argc)++;
	va_end(copy);

	char **ss = vcalloc(*argc + 1, sizeof(char*));
	*argc = 0;
	while ((s = va_arg(vl, char*)))
		ss[(*argc)++] = s;

	return ss;
}

static pid_t fork_zpush(int fd, ...)
{
	va_list ap;
	char **argv;
	int argc;

	va_start(ap, fd);
	argv = vargv(ap, &argc);
	va_end(ap);

	pid_t zpush_pid = fork();
	if (zpush_pid < 0)
		BAIL_OUT("failed to fork zpush child process!");

	if (zpush_pid > 0)
		return zpush_pid;

	dup2(fd, 0);
	for (fd = 3; fd < 1024; fd++)
		close(fd);

	exit(ztk_push(argc, argv));
}

static pid_t fork_zpull(int fd, ...)
{
	va_list ap;
	char **argv;
	int argc;

	va_start(ap, fd);
	argv = vargv(ap, &argc);
	va_end(ap);

	pid_t zpull_pid = fork();
	if (zpull_pid < 0)
		BAIL_OUT("failed to fork zpull child process!");

	if (zpull_pid > 0)
		return zpull_pid;

	dup2(fd, 1);
	for (fd = 3; fd < 1024; fd++)
		close(fd);

	exit(ztk_pull(argc, argv));
}
