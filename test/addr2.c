int main()
{
	int test;
	int *addr;
	int **addr_ptr;
	test = 15;
	addr = &test;
	addr_ptr = &addr;
	return **addr_ptr;
}
