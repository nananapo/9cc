void put_charptrv(void *ptr);
void put_strptrv(void *ptr);
void put_intptrv(void *ptr);

int main(void)
{
	int a;
	char b;
	char *c;

	a = 100000;
	b = 120;
	c = "HeySayJump";
	
	put_intptrv((void *)&a);
	put_charptrv((void *)&b);
	put_strptrv((void *)&c);
}
