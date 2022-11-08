#include <stdio.h>
 
int a = 10000;
int c = 8400;
int b;
int d;
int e;
int g;
int f[8401];
 
int main(void) {
  for (b = 0; b < c; b++) {
    f[b] = a / 5;
  }
 
  e = 0;
  for (c = 8400; c > 0; c -= 14) {
    d = 0;
    for (b = c - 1; b > 0; b--) {
      g = 2 * b - 1;
      d = d * b + f[b] * a;
      f[b] = d % g;
      d /= g;
    }
    printf("%04d", e + d / a);
    e = d % a;
  }
  return 0;
}
