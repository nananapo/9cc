#!/bin/bash

prefix="
int pint(int i);
int dint(int i);
int pchar(char a);
int pspace(int n);
int pline();
int my_putstr(char *s, int n);
int my_print(char *s);
int *my_malloc_int(int n);
"

testdir="../test/"
ncc="./9cc"
prpr="../prpr/prpr --stddir ../std/"
module="$testdir/sub/print.c"

assert() {
  expected="$1"
  input="$2"

  echo "$input" | $prpr - | $ncc > tmp.s
  if [ "$?" != "0" ]; then
	echo "$input => FAILED TO COMPILE"
	exit 1
  fi

  cc -o tmp tmp.s
  if [ "$?" != "0" ]; then
	echo "$input => GCC FAIL"
	exit 1
  fi

  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "ASSERT $input => $expected expected, but got $actual"
    exit 1
  fi
}

assert_out(){
	echo "#-- Test----------------------------"

	arg2="$2"

	expected="$1"
	input="$prefix$2"
	
    HEY="`echo "$input" | $prpr -`"
	echo "$HEY"| $ncc > tmp.s
	cc -o tmp tmp.s $module
	actual=`./tmp`

  	if [ "$actual" = "$expected" ]; then
  	  echo "$arg2 => $actual"
  	else
  	  echo "ASSERT OUT $input => $expected expected, but got $actual"
  	  exit 1
  	fi
}

actualfile="actual.output"
expectedfile="expected.output"
tmpcfile="$testdir/tmp.c"

assert_gcc(){
	input="$prefix`cat $testdir/$1`"
	
    echo "$input" | $prpr - | $ncc > tmp.s
	if [ "$?" != "0" ]; then
		echo "$1 => 9cc KO"
		exit 1
	fi
	
	cc -o tmp1 tmp.s $module
	if [ "$?" != "0" ]; then
		echo "$1 => 9cc gcc compile KO"
		exit 1
	fi

	./tmp1 > $actualfile
	actual=`cat -e $actualfile`

	echo "$input" > $tmpcfile
	cc -o tmp2 $tmpcfile $module
	if [ "$?" != "0" ]; then
		rm -rf $tmpcfile
		echo "$1 => gcc segv KO"
		exit 1
	fi

	./tmp2 > $expectedfile
	expected=`cat -e $expectedfile`

	rm -rf $tmpcfile tmp1 tmp2

  	if [ "$actual" = "$expected" ]; then
  	  echo "$1 => OK"
  	else
  	  echo "ASSERT GCC $1 => KO"
  	  exit 1
  	fi
}

assert 0 "int main(){0;}"
assert 42 "int main(){42;}"

assert 42 "int main(){11+12+19;}"
assert 1 "int main(){11+12+19-41;}"

assert 1 "int main(){11 - 41 + 19 + 12;}"
assert 55 "int main(){1 +  2 +   3 + 4+5 +  6 + 7    +8+9 + 10;}"

assert 26 "int main(){2*3+4*5;}"
assert 70 "int main(){2*(3+4)*5;}"
assert 1 "int main(){((1));}"

assert 3 "int main(){(15 + 3) / 6;}"

assert 5 "int main(){+5;}"
assert 10 "int main(){5 - -5;}"
assert 48 "int main(){12 * -2 * -2;}"

assert 1 "int main(){3 + 5 == 8;}"
assert 0 "int main(){3 + 4 == 8;}"
assert 0 "int main(){3 + 5 != 8;}"
assert 1 "int main(){3 + 4 != 8;}"
assert 1 "int main(){-3 + 2 < 0;}"
assert 0 "int main(){-3 + 2 > 0;}"
assert 1 "int main(){-3 + 3 <= 0;}"

assert 3 "int main(){int a;a=3;}"
assert 2 "int main(){int z;z=2;}"
assert 1 "int main(){int a;int b;a=b=1;}"
assert 10 "int main(){int a;int b;a=b=10;a;}"
assert 10 "int main(){int a;int b;a=b=10;b;}"
assert 0 "int main(){int a;int b;a=1;b=2;a>b;}"
assert 1 "int main(){int a;int b;a=1;b=2;a<b;}"

assert 3 "int main(){int abc;abc=3;}"
assert 2 "int main(){int zab;zab=2;}"
assert 1 "int main(){int abc;int def;abc=def=1;}"
assert 10 "int main(){int abc;int def;abc=def=10;abc;}"
assert 10 "int main(){int abc;int def;abc=def=10;def;}"
assert 0 "int main(){int abc;int def;abc=1;def=2;abc>def;}"
assert 1 "int main(){int abc;int def;abc=1;def=2;abc<def;}"

assert 10 "int main(){return 10;}"
assert 15 "int main(){int a;a = 15;return a;}"
assert 20 "int main(){int a;int b;int c;a = 3; b = 5; c = 12; return a + b + c;}"

assert 2 "int main(){if (1) return 2;}"
assert 2 "int main(){if (0) return 3; else return 2;}"
assert 2 "int main(){int a;a = 1; if (a) return 2;}"
assert 2 "int main(){int a;int b;a = 1; b = 3; if (a < b) return 2; else return 3;}"
assert 3 "int main(){int a;int b;a = 3; b = 1; if (a < b) return 2; else return 3;}"

assert 10 "int main(){int i;int s;i=0;s=0;while(i<10) s = s + (i = i + 1);return 10;}"
assert 55 "int main(){int i;int s;i=0;s=0;while(i<10) s = s + (i = i + 1);return s;}"

assert 10 "int main(){int s;int a;s = 0;for(a=0;a<=10;a=a+1) s = s + a; return 10;}"
assert 55 "int main(){int s;int a;s = 0;for(a=0;a<=10;a=a+1) s = s + a; return s;}"

assert 10 "int main(){{return 10;}}"
assert 10 "int main(){{int a;int b;a = 3; b = 7;return a+b;}}"
assert 55 "int main(){int a;int s;a = 1; s = 0;for (;;) { s = s + a; a = a + 1; if (a == 11) { return s; }}}"

assert_out "1" "int main(){int i;pint(1);}"
assert_out "1" "int main(){pint(1);}"

assert_out "22" "int called(int a) { pint(a); }
int main(){ called(22); }"

assert_out "123" "int called(int a, int b) { pint(a + b); }
int main(){ called(22, 101); }"

assert_gcc "fib.c"

assert 10 "int main()
{
	int test;
	int *addr;
	test = 10;
	addr = &test;
	return *addr;
}"

assert 15 "int main()
{
	int test;
	int *addr;
	int **addr_ptr;
	test = 15;
	addr = &test;
	addr_ptr = &addr;
	return **addr_ptr;
}"

assert 10 "int add(int a, int b)
{
	return a + b;
}
int main()
{
	return add(4,6);
}"

assert_gcc "add6.c"

assert 0 "int main(){int a; int *aa;int **aaa;return 0;}"
assert 0 "int main(){return 0;}int *sub(){}int *sub2(int *a,int **b){}"

assert 3 "int main()
{
	int x;
	int *y;
	y = &x;
	*y = 3;
	return x;
}"

assert 42 "int main()
{
	int x;
	int *y;
	int **z;
	x = 3;
	y = &x;
	z = &y;
	**z = 42;
	return x;
}"

assert_out "42 24" "int swap(int *x, int *y)
{
	int tmp;
	tmp = *x;
	*x = *y;
	*y = tmp;
	return 0;
}

int main()
{
	int x;
	int y;
	x = 24;
	y = 42;
	swap(&x, &y);
	pint(x);
	pspace(1);
	pint(y);
	return 0;
}"

assert_out "42 24" "int	main()
{
	int *ptr;
	ptr = my_malloc_int(2);
	*ptr = 42;
	*(ptr + 1) = 24;
	pint(*ptr);
	pspace(1);
	pint(*(ptr + 1));
	return 0;
}"
assert_out "42 24" "int	main()
{
	int *ptr;
	ptr = my_malloc_int(2);
	*ptr = 42;
	*(1 + ptr) = 24;
	pint(*ptr);
	pspace(1);
	pint(*(1 + ptr));
	return 0;
}"

assert 4 "int main(){return sizeof 1;}"
assert 4 "int main(){return sizeof(1);}"
assert 4 "int main(){int a;return sizeof(a);}"
assert 28 "int main(){ int a[7]; return sizeof(a);}"
assert 84 "int main(){ int a[7][3]; return sizeof(a);}" 
assert 168 "int main(){ int *a[7][3]; return sizeof(a);}" 

assert 1 "int main(){char a; return sizeof(a);}";
assert 19 "int main(){char a[19]; return sizeof(a);}"
assert 57 "int main(){char a[19][3]; return sizeof(a);}"
assert 120 "int main(){char *a[5][3]; return sizeof(a);}"

assert 8 "int main(){int *a;return sizeof(a);}"
assert 8 "int main(){int a;return sizeof(&a);}"

assert 1 "int main() {return sizeof(char); }"
assert 4 "int main() {return sizeof(int); }"
assert 8 "int main() {return sizeof(int *); }"
assert 8 "int main() {return sizeof(int **); }"
assert 8 "int main() {return sizeof(char *); }"

assert 10 "int main(){int a[3]; int *b; b = a; *b = 10; return *a;}"
assert 33 "int main(){int a[3]; *a = 10; *(a + 1) = 11; *(a + 2) = 12; return (*a + *(a + 1) + *(a + 2));}"
assert 10 "int main(){int a[3]; *a = 10; *(a + 1) = 11; *(a + 2) = 12; return *a;}"
assert 11 "int main(){int a[3]; *a = 10; *(a + 1) = 11; *(a + 2) = 12; return *(a + 1);}"
assert 12 "int main(){int a[3]; *a = 10; *(a + 1) = 11; *(a + 2) = 12; return *(a + 2);}"
assert 33 "int main(){int a[3]; int b; int c; *a = 10; *(a + 1) = 11; *(a + 2) = 12; return (*a + *(a + 1) + *(a + 2));}"

assert 1 "int main(){int a[3][3]; **a=1;*(*a+1)=2;*(*a+2)=3; **(a+1)=4;*(*(a+1)+1)=5;*(*(a+1)+2)=6; **(a+2)=7;*(*(a+2)+1)=8;*(*(a+2)+2)=9; return **a;}"
assert 2 "int main(){int a[3][3]; **a=1;*(*a+1)=2;*(*a+2)=3; **(a+1)=4;*(*(a+1)+1)=5;*(*(a+1)+2)=6; **(a+2)=7;*(*(a+2)+1)=8;*(*(a+2)+2)=9; return *(*a+1);}"
assert 3 "int main(){int a[3][3]; **a=1;*(*a+1)=2;*(*a+2)=3; **(a+1)=4;*(*(a+1)+1)=5;*(*(a+1)+2)=6; **(a+2)=7;*(*(a+2)+1)=8;*(*(a+2)+2)=9; return *(*a+2);}"
assert 4 "int main(){int a[3][3]; **a=1;*(*a+1)=2;*(*a+2)=3; **(a+1)=4;*(*(a+1)+1)=5;*(*(a+1)+2)=6; **(a+2)=7;*(*(a+2)+1)=8;*(*(a+2)+2)=9; return **(a+1);}"
assert 5 "int main(){int a[3][3]; **a=1;*(*a+1)=2;*(*a+2)=3; **(a+1)=4;*(*(a+1)+1)=5;*(*(a+1)+2)=6; **(a+2)=7;*(*(a+2)+1)=8;*(*(a+2)+2)=9; return *(*(a+1)+1);}"
assert 6 "int main(){int a[3][3]; **a=1;*(*a+1)=2;*(*a+2)=3; **(a+1)=4;*(*(a+1)+1)=5;*(*(a+1)+2)=6; **(a+2)=7;*(*(a+2)+1)=8;*(*(a+2)+2)=9; return *(*(a+1)+2);}"
assert 7 "int main(){int a[3][3]; **a=1;*(*a+1)=2;*(*a+2)=3; **(a+1)=4;*(*(a+1)+1)=5;*(*(a+1)+2)=6; **(a+2)=7;*(*(a+2)+1)=8;*(*(a+2)+2)=9; return **(a+2);}"
assert 8 "int main(){int a[3][3]; **a=1;*(*a+1)=2;*(*a+2)=3; **(a+1)=4;*(*(a+1)+1)=5;*(*(a+1)+2)=6; **(a+2)=7;*(*(a+2)+1)=8;*(*(a+2)+2)=9; return *(*(a+2)+1);}"
assert 9 "int main(){int a[3][3]; **a=1;*(*a+1)=2;*(*a+2)=3; **(a+1)=4;*(*(a+1)+1)=5;*(*(a+1)+2)=6; **(a+2)=7;*(*(a+2)+1)=8;*(*(a+2)+2)=9; return *(*(a+2)+2);}"

assert 1 "int main(){ int a[3]; a[0] = 1; return a[0];}"
assert 1 "int main(){ int a[3]; a[0] = 1; a[1] = 10; return a[0];}"
assert 10 "int main(){ int a[3]; a[0] = 1; a[1] = 10; return a[1];}"
assert 100 "int main(){ int a[3]; a[0] = 1; a[1] = 10; a[2] = 100; return a[2];}"

assert 1 "int main(){ int a[3]; 0[a] = 1; return a[0];}"
assert 1 "int main(){ int a[3]; 0[a] = 1; 1[a] = 10; return a[0];}"
assert 10 "int main(){ int a[3]; 0[a] = 1; 1[a] = 10; return a[1];}"
assert 100 "int main(){ int a[3]; 0[a] = 1; 1[a] = 10; 2[a] = 100; return a[2];}"

assert 1 "int main(){ int a[3]; 0[a] = 1; return 0[a];}"
assert 1 "int main(){ int a[3]; 0[a] = 1; 1[a] = 10; return 0[a];}"
assert 10 "int main(){ int a[3]; 0[a] = 1; 1[a] = 10; return 1[a];}"
assert 100 "int main(){ int a[3]; 0[a] = 1; 1[a] = 10; 2[a] = 100; return 2[a];}"

assert_out "0 0 1 3 2 6 " "int main(){ int i; int j; j = 0; for (i=0; i<3; i=i+1){ pint(i); pspace(1); pint(j); pspace(1); j = j + 3;} return 0;}"

assert 1 "int main(){ int a[3][3]; int i; int j; for (i=0; i<3; i=i+1) for (j=0; j<3; j=j+1) a[i][j] = i * 3 + (j+1); return a[0][0];}"
assert 2 "int main(){ int a[3][3]; int i; int j; for (i=0; i<3; i=i+1) for (j=0; j<3; j=j+1) a[i][j] = i * 3 + (j+1); return a[0][1];}"
assert 3 "int main(){ int a[3][3]; int i; int j; for (i=0; i<3; i=i+1) for (j=0; j<3; j=j+1) a[i][j] = i * 3 + (j+1); return a[0][2];}"
assert 4 "int main(){ int a[3][3]; int i; int j; for (i=0; i<3; i=i+1) for (j=0; j<3; j=j+1) a[i][j] = i * 3 + (j+1); return a[1][0];}"
assert 5 "int main(){ int a[3][3]; int i; int j; for (i=0; i<3; i=i+1) for (j=0; j<3; j=j+1) a[i][j] = i * 3 + (j+1); return a[1][1];}"
assert 6 "int main(){ int a[3][3]; int i; int j; for (i=0; i<3; i=i+1) for (j=0; j<3; j=j+1) a[i][j] = i * 3 + (j+1); return a[1][2];}"
assert 7 "int main(){ int a[3][3]; int i; int j; for (i=0; i<3; i=i+1) for (j=0; j<3; j=j+1) a[i][j] = i * 3 + (j+1); return a[2][0];}"
assert 8 "int main(){ int a[3][3]; int i; int j; for (i=0; i<3; i=i+1) for (j=0; j<3; j=j+1) a[i][j] = i * 3 + (j+1); return a[2][1];}"
assert 9 "int main(){ int a[3][3]; int i; int j; for (i=0; i<3; i=i+1) for (j=0; j<3; j=j+1) a[i][j] = i * 3 + (j+1); return a[2][2];}"


assert 0 "int a; int main() { return a; }"
assert 100 "int a; int main() { a = 100; return a;}"
assert 127 "int a; int main() { a = a + 127; return a; }"
assert 111 "int a; int main() { a = 25; int a; a = 111; return a; }"

assert 1 "int a[3]; int main() { int i; for(i = 0; i < 3; i = i + 1) a[i] = i + 1; return a[0]; }"
assert 2 "int a[3]; int main() { int i; for(i = 0; i < 3; i = i + 1) a[i] = i + 1; return a[1]; }"
assert 3 "int a[3]; int main() { int i; for(i = 0; i < 3; i = i + 1) a[i] = i + 1; return a[2]; }"

assert 10 "int *a[3]; int main() { int i; int j; int k; a[0] = &i; a[1] = &j; a[2] = &k; i = 10; j = 20; k = 30; return *a[0];}"
assert 20 "int *a[3]; int main() { int i; int j; int k; a[0] = &i; a[1] = &j; a[2] = &k; i = 10; j = 20; k = 30; return *a[1];}"
assert 30 "int *a[3]; int main() { int i; int j; int k; a[0] = &i; a[1] = &j; a[2] = &k; i = 10; j = 20; k = 30; return *a[2];}"

assert 54 "int main(){char a; char b; a = 54; b = 32; return a;}"
assert 32 "int main(){char a; char b; a = 54; b = 32; return b;}"
assert 54 "int main(){char a; char b; b = 32; a = 54; return a;}"
assert 32 "int main(){char a; char b; b = 32; a = 54; return b;}"

assert 124 "int main(){char a[3]; a[0] = 124; a[1] = 110; a[2] = 99; return a[0];}"
assert 110 "int main(){char a[3]; a[0] = 124; a[1] = 110; a[2] = 99; return a[1];}"
assert 99 "int main(){char a[3]; a[0] = 124; a[1] = 110; a[2] = 99; return a[2];}"

assert 54 "char a; char b; int main() { a = 54; b = 32; return a; }"
assert 32 "char a; char b; int main() { a = 54; b = 32; return b; }"
assert 54 "char a; char b; int main() { b = 32; a = 54; return a; }"
assert 32 "char a; char b; int main() { b = 32; a = 54; return b; }"

assert 124 "int main(){char a[3]; a[2] = 124; a[1] = 110; a[0] = 99; return a[2];}"
assert 110 "int main(){char a[3]; a[2] = 124; a[1] = 110; a[0] = 99; return a[1];}"
assert 99 "int main(){char a[3]; a[2] = 124; a[1] = 110; a[0] = 99; return a[0];}"

assert 124 "char a[3]; int main() { a[0] = 124; a[1] = 1; a[2] = 0; return a[a[2]]; }"
assert 1 "char a[3]; int main() { a[0] = 124; a[1] = 1; a[2] = 0; return a[a[1]]; }"

assert_out "kanapo" "int main() {
	char a[6];
	a[0] = 107;
	a[1] = 97;
	a[2] = 110;
	a[3] = 97;
	a[4] = 112;
	a[5] = 111;
	my_putstr(a, 6);
}"

assert_out "kanapo" "int main() {
	char a[7];
	a[0] = 107;
	a[1] = 97;
	a[2] = 110;
	a[3] = 97;
	a[4] = 112;
	a[5] = 111;
	a[6] = 0;
	my_print(a);
}"

assert_out "kanapo" "int main() {
	char a[7];
	a[0] = 107;
	a[3] = 97;
	a[5] = 111;
	a[4] = 112;
	a[6] = 0;
	a[2] = 110;
	a[1] = 97;
	my_print(a);
}"

assert_out "kanapo" "char a[7];
int main() {
	a[0] = 107;
	a[1] = 97;
	a[2] = 110;
	a[3] = 97;
	a[4] = 112;
	a[5] = 111;
	a[6] = 0;
	my_print(a);
}"

assert_out "kanapo" "char a[7];
int main() {
	a[5] = 111;
	a[1] = 97;
	a[2] = 110;
	a[6] = 0;
	a[4] = 112;
	a[0] = 107;
	a[3] = 97;
	my_print(a);
}"

assert_out "HelloWorld" "int printf(char *a);
int main()
{
	printf(\"HelloWorld\");
}"

assert_out "HelloWorld" "int printf(char *a);
int main()
{
	char *s;
	s = \"HelloWorld\";
	printf(s);
}"

assert_out "HelloWorld" "int printf(char *a);
char *s;
int main()
{
	s = \"HelloWorld\";
	printf(s);
}"

assert_out "HelloWorld" "int printf(char *a);
char *s;
int main()
{
	s = \"Hello\";
	printf(s);
	s = \"World\";
	printf(s);
}"

assert_out "1" "int main()
{
	pint(-(-1));
}"

assert_out "1" "int main()
{
	int n;

	n = -1;
	pint(-n);
}"

assert_out "Hello" "int printf(char *a);
int main()
{
	char	*s;
	s = \"Hello\\n\";
	printf(s);
}"

assert "127" "int main(){
	char a;
	int i;
	a = 127;
	i = 222;
	return a;
}"

assert "222" "int main(){
	char a;
	int i;
	a = 127;
	i = 222;
	return i;
}"

assert_gcc "8queen.c"
assert_gcc "9queen.c"

assert_gcc "structsize1.c"
assert_gcc "structsize2.c"
assert_gcc "structsize3.c"
assert_gcc "structsize4.c"
assert_gcc "structsize5.c"

assert_gcc "struct2.c"
assert_gcc "struct3.c"
assert_gcc "struct4.c"
assert_gcc "struct5.c"
assert_gcc "struct6.c"
assert_gcc "struct7.c"

assert_gcc "arg1.c"
assert_gcc "arg2.c"
assert_gcc "arg3.c"
assert_gcc "arg4.c"
assert_gcc "arg5.c"
assert_gcc "arg6.c"
assert_gcc "arg7.c"
assert_gcc "arg8.c"
assert_gcc "arg9.c"

assert_gcc "varg.c"
assert_gcc "varg2.c"
assert_gcc "varg3.c"

assert_gcc "str1.c"

assert "125" "struct t1 {char a;};
int main()
{
	struct t1 s;
	s.a = 125;
	return s.a;
}"

assert "101" "struct t1 {char a; char b;};
int main()
{
	struct t1 s;
	s.a = 101;
	return s.a;
}"

assert "102" "struct t1 {char a; char b;};
int main()
{
	struct t1 s;
	s.b = 102;
	return s.b;
}"

assert "55" "struct t1 {char a; char b;};
int main()
{
	struct t1 s;
	s.a = 55;
	s.b = 110;
	return s.a;
}"

assert "110" "struct t1 {char a; char b;};
int main()
{
	struct t1 s;
	s.a = 55;
	s.b = 110;
	return s.b;
}"

assert "55" "struct t1 {char a; char b;};
int main()
{
	struct t1 s;
	s.b = 110;
	s.a = 55;
	return s.a;
}"

assert "110" "struct t1 {char a; char b;};
int main()
{
	struct t1 s;
	s.b = 110;
	s.a = 55;
	return s.b;
}"

assert "112" "struct t1 {int a; char b; int c;};
int main()
{
	struct t1 s;
	struct t1 *p;
	p = &s;
	p->c = 65535;
	p->a = 65435;
	p->b = 12;
	return p->c - p->a + p->b;
}" 

assert "115" "struct t1 {int a; char b; int c;};
int main()
{
	struct t1 s;
	struct t1 *p;
	p = &s;
	p->c = 2147483647;
	p->a = 2147483547;
	p->b = 15;
	return p->c - p->a + p->b;
}"

assert_gcc "void1.c"
assert_gcc "void2.c"

assert_gcc "cast0.c"
assert_gcc "cast1.c"
assert_gcc "cast2.c"

assert_gcc "implicit_cast0.c"

assert_gcc "cond1.c"

assert_gcc "ctype.c"

assert_gcc "increments1.c"
assert_gcc "increments2.c"
assert_gcc "increments3.c"
assert_gcc "increments4.c"
assert_gcc "increments5.c"
assert_gcc "increments6.c"
assert_gcc "increments7.c"
assert_gcc "increments8.c"

assert_gcc "not.c"

assert_gcc "k_r_strcpy.c"

assert_gcc "dowhile.c"
assert_gcc "return.c"

assert_gcc "continue1.c"
assert_gcc "continue2.c"
assert_gcc "continue3.c"

assert_gcc "break1.c"
assert_gcc "break2.c"
assert_gcc "break3.c"

assert_gcc "nostmt1.c"

assert_gcc "compound_assignment1.c"

assert_gcc "mod.c"

assert_gcc "declare_assign.c"

assert_gcc "switch1.c"
assert_gcc "switch2.c"
assert_gcc "switch3.c"

assert_gcc "duffsdevice.c"

assert_gcc "typedef1.c"
assert_gcc "defstruct1.c"
assert_gcc "defstruct2.c"

assert_gcc "elseif.c"

assert_gcc "enum1.c"
assert_gcc "enum2.c"
assert_gcc "enum3.c"
assert_gcc "enum4.c"
assert_gcc "enum5.c"
assert_gcc "enum6.c"

assert_gcc "size1.c"
assert_gcc "size2.c"
assert_gcc "size3.c"
assert_gcc "size4.c"

assert_gcc "stack.c"

assert_gcc "global1.c"
assert_gcc "global2.c"
assert_gcc "global3.c"
assert_gcc "global4.c"
assert_gcc "global5.c"
assert_gcc "global6.c"
assert_gcc "global7.c"
assert_gcc "global8.c"

echo OK
