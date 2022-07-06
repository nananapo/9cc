#ifndef STDIO_H
# define STDIO_H

void	perror(char *s);
void	printf(char *fmt, ...);
void	fprintf(void *fp, char *fmt, ...);
void	*fopen(char *path, char *mode);

# define stdin fopen("/dev/stdin", "r")
# define stdout fopen("/dev/stdout", "a")
# define stderr fopen("/dev/stderr", "a")

#endif
