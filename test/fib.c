int pint(int x);
int pspace(int x);

int fib(int x)
{
	if (x == 0)
		return 1;
	if (x == 1)
		return 1;
	return fib(x-2) + fib(x-1);
}
int main(){
	int i;
	for(i=0;i<10;i=i+1)
	{
		int x;
		x = fib(i);
		pint(x);
		pspace(1);
	}
}
