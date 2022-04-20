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

assert 0 0
assert 42 42
assert 42 "11+12+19"
assert 1 "11+12+19-41"
assert 1 "11 - 41 + 19 + 12"
assert 55 "1 +  2 +   3 + 4+5 +  6 + 7    +8+9 + 10"
echo OK
