int *a[3]; int main() { int i; int j; int k; a[0] = &i; a[1] = &j; a[2] = &k; i = 10; j = 20; k = 30; return *a[2];}
