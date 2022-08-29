#include <stdio.h>

char	g_buf[60000];

char	*g_ptr;
char	*g_program;

void	loop(void)
{
	int	i;

	switch (*g_program)
	{
		case '>': g_ptr++;			break ;
		case '<': g_ptr--;			break ;
		case '+': (*g_ptr)++;printf("%d\n", *g_ptr);	break ;
		case '-': (*g_ptr)--;		break ;
		case '.': putchar(*g_ptr);	break ;
		case ',': *g_ptr=getchar();	break ;
		case '[':
			i = 0;
			while (*g_ptr == 0 && *(++g_program) != '\0' && !(i == 0 && *g_program == ']'))
			{
				printf("next %s\n", g_program);
				if (*g_program == '[') i++;
				else if (*g_program == ']') i--;
			}
			break ;
		case ']':
			i = 0;
			while (*g_ptr != 0 && *(--g_program) != '\0' && !(i == 0 && *g_program == '['))
			{
				printf("back %s\n", g_program);
				if (*g_program == ']') i++;
				else if (*g_program == '[') i--;
			}
			break ;
	}
	if (*g_program != '\0')
		g_program++;
}

int	main(int argc, char **argv)
{
	char	*ptr;

	if (argc != 2)
	{
		printf("Usage: brainfuck program\n");
		return (1);
	}
	g_ptr = g_buf + 30000;
	g_program = argv[1];
	while (*g_program != '\0')
		loop();
}
