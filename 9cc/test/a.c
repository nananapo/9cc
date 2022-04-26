int pint(int i);
int pspace(int n);
int *my_malloc_int(int n);

int main()
{
	int *ptr;
	ptr = my_malloc_int(2);
	*ptr = 42;
	pint(*pstr);
}
