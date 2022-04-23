#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}


assert_out(){
	expected="$1"
	input="$2"
	
	./9cc "$input" > tmp.s
	cc -o tmp tmp.s "test/print.c"
	actual=`./tmp`
	
  	if [ "$actual" = "$expected" ]; then
  	  echo "$input => $actual"
  	else
  	  echo "$input => $expected expected, but got $actual"
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

assert_out "1" "int main(){pint(1);}"
assert_out "1 1 2 3 5 8 13 21 34 55 " "int fib(int x)
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
}"

assert 10 "int main()
{
	int test;
	int addr;
	test = 10;
	addr = &test;
	return *addr;
}"
assert 10 "int main()
{
	int test;
	int addr;
	int addr_ptr;
	test = 10;
	addr = &test;
	addr_ptr = &addr;
	return **addr_ptr;
}"
echo OK
