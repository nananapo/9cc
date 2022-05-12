int add6(int a, int b, int c, int d , int e, int f) return a + b + c + d + e + f;

int main()
	return add6(1,2,3,4,5,add6(1,2,3,4,5,add6(1,2,3,4,5,6)));
