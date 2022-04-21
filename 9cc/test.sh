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

assert 0 "0;"
assert 42 "42;"

assert 42 "11+12+19;"
assert 1 "11+12+19-41;"

assert 1 "11 - 41 + 19 + 12;"
assert 55 "1 +  2 +   3 + 4+5 +  6 + 7    +8+9 + 10;"

assert 26 "2*3+4*5;"
assert 70 "2*(3+4)*5;"
assert 1 "((1));"

assert 3 "(15 + 3) / 6;"

assert 5 "+5;"
assert 10 "5 - -5;"
assert 48 "12 * -2 * -2;"

assert 1 "3 + 5 == 8;"
assert 0 "3 + 4 == 8;"
assert 0 "3 + 5 != 8;"
assert 1 "3 + 4 != 8;"
assert 1 "-3 + 2 < 0;"
assert 0 "-3 + 2 > 0;"
assert 1 "-3 + 3 <= 0;"

assert 3 "a=3;"
assert 2 "z=2;"
assert 1 "a=b=1;"
assert 10 "a=b=10;a;"
assert 10 "a=b=10;b;"
assert 0 "a=1;b=2;a>b;"
assert 1 "a=1;b=2;a<b;"

assert 3 "abc=3;"
assert 2 "zab=2;"
assert 1 "abc=def=1;"
assert 10 "abc=def=10;abc;"
assert 10 "abc=def=10;def;"
assert 0 "abc=1;def=2;abc>def;"
assert 1 "abc=1;def=2;abc<def;"

assert 10 "return 10;"
assert 15 "a = 15;return a;"
assert 20 "a = 3; b = 5; c = 12; return a + b + c;"

assert 2 "if (1) return 2;"
assert 2 "if (0) return 3; else 2;"
assert 2 "a = 1; if (a) return 2;"
assert 2 "a = 1; b = 3; if (a < b) return 2; else return 3;"
assert 3 "a = 3; b = 1; if (a < b) return 2; else return 3;"

assert 55 "i=0;s=0;while(i<10) s = s + (i = i + 1);return s;"
echo OK
