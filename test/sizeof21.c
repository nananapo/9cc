#include <stdio.h>
#include <stdbool.h>

typedef struct s_sbdata
{
	bool	isswitch;

	int		startlabel;
	int		endlabel;

	void	*type;
	void	*cases;

	int		defaultLabel;
}	SBData;

int main(void)
{
	printf("%lu\n", sizeof(SBData));
}
