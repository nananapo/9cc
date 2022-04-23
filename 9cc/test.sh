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

assert 0 "main(){0;}"
assert 42 "main(){42;}"

assert 42 "main(){11+12+19;}"
assert 1 "main(){11+12+19-41;}"

assert 1 "main(){11 - 41 + 19 + 12;}"
assert 55 "main(){1 +  2 +   3 + 4+5 +  6 + 7    +8+9 + 10;}"

assert 26 "main(){2*3+4*5;}"
assert 70 "main(){2*(3+4)*5;}"
assert 1 "main(){((1));}"

assert 3 "main(){(15 + 3) / 6;}"

assert 5 "main(){+5;}"
assert 10 "main(){5 - -5;}"
assert 48 "main(){12 * -2 * -2;}"

assert 1 "main(){3 + 5 == 8;}"
assert 0 "main(){3 + 4 == 8;}"
assert 0 "main(){3 + 5 != 8;}"
assert 1 "main(){3 + 4 != 8;}"
assert 1 "main(){-3 + 2 < 0;}"
assert 0 "main(){-3 + 2 > 0;}"
assert 1 "main(){-3 + 3 <= 0;}"

assert 3 "main(){a=3;}"
assert 2 "main(){z=2;}"
assert 1 "main(){a=b=1;}"
assert 10 "main(){a=b=10;a;}"
assert 10 "main(){a=b=10;b;}"
assert 0 "main(){a=1;b=2;a>b;}"
assert 1 "main(){a=1;b=2;a<b;}"

assert 3 "main(){abc=3;}"
assert 2 "main(){zab=2;}"
assert 1 "main(){abc=def=1;}"
assert 10 "main(){abc=def=10;abc;}"
assert 10 "main(){abc=def=10;def;}"
assert 0 "main(){abc=1;def=2;abc>def;}"
assert 1 "main(){abc=1;def=2;abc<def;}"

assert 10 "main(){return 10;}"
assert 15 "main(){a = 15;return a;}"
assert 20 "main(){a = 3; b = 5; c = 12; return a + b + c;}"

assert 2 "main(){if (1) return 2;}"
assert 2 "main(){if (0) return 3; else return 2;}"
assert 2 "main(){a = 1; if (a) return 2;}"
assert 2 "main(){a = 1; b = 3; if (a < b) return 2; else return 3;}"
assert 3 "main(){a = 3; b = 1; if (a < b) return 2; else return 3;}"

assert 10 "main(){i=0;s=0;while(i<10) s = s + (i = i + 1);return 10;}"
assert 55 "main(){i=0;s=0;while(i<10) s = s + (i = i + 1);return s;}"

assert 10 "main(){s = 0;for(a=0;a<=10;a=a+1) s = s + a; return 10;}"
assert 55 "main(){s = 0;for(a=0;a<=10;a=a+1) s = s + a; return s;}"

assert 10 "main(){{return 10;}}"
assert 10 "main(){{a = 3; b = 7;return a+b;}}"
assert 55 "main(){a = 1; s = 0;for (;;) { s = s + a; a = a + 1; if (a == 11) { return s; }}}"
echo OK
