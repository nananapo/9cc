int my_putchar(char a);
int my_print(char *a);

int main()
{
	char a;
	char *b;

	a = '\\';
	my_putchar(a);
	b = "\"\'HelloWorld!\\\"\'";
	my_print(b);
}
