#ifndef STDIO_H
# define STDIO_H

void	perror(char *s);
int		printf(char *fmt, ...);
int		fprintf(void *fp, char *fmt, ...);
void	*fopen(char *path, char *mode);
int		sprintf(char *str, char *fmt, ...);

extern	void *__stdinp;
extern	void *__stdoutp;
extern	void *__stderrp;

# define stdin __stdinp
# define stdout __stdoutp
# define stderr __stderrp

#endif
