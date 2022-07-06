#ifndef UNISTD_H
# define UNISTD_H

# include <stddef.h>

int open(char *pathname, int flags);
int close(int fildes);

int	read(int fd, char *buf, size_t count);

#endif
