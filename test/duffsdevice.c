#include <string.h>

int	my_print(char *c);
int	my_putchar(char c);
int	dint(int c);

char	*duff_strcpy(char *to, char *from)
{
	char	*ret = to;
	int		count = (int)strlen(from) + 1;
	switch (count % 8) {
		case 0:		do {	my_putchar(*from); *to++ = *from++; dint(to - ret);
		case 7:				my_putchar(*from); *to++ = *from++; dint(to - ret);
		case 6:				my_putchar(*from); *to++ = *from++; dint(to - ret);
		case 5:				my_putchar(*from); *to++ = *from++; dint(to - ret);
		case 4:				my_putchar(*from); *to++ = *from++; dint(to - ret);
		case 3:				my_putchar(*from); *to++ = *from++; dint(to - ret);
		case 2:				my_putchar(*from); *to++ = *from++; dint(to - ret);
		case 1:				my_putchar(*from); *to++ = *from++; dint(to - ret);
					} while ((count -= 8) > 0);
	}
	return (ret);
}

int main(void)
{
	char s1[1000];
	char s2[1000];

	strcpy(s1, "HelloWorld!1\n");
	my_print(s1);

	duff_strcpy(s2, "HelloWorld!2\n");
	my_print(s2);
}
