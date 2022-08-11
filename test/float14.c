void putf(float a);
void printf(char *fmt, ...);
int main(void)
{
	float a = 0.02;
	float b = 0.04;
	putf(a);
	putf(b);
	printf("%d\n", a < b);
	printf("%d\n", a > b);
}
