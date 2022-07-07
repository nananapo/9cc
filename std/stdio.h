#ifndef STDIO_H
# define STDIO_H

void	perror(char *s);
void	printf(char *fmt, ...);
void	fprintf(void *fp, char *fmt, ...);
void	*fopen(char *path, char *mode);

extern	void *__stdinp;
extern	void *__stdoutp;
extern	void *__stderrp;

# define stdin __stdinp
# define stdout __stdoutp
# define stderr __stderrp

#endif
