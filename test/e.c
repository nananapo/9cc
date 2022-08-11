#include <stdio.h>

int*p;
int w[2048];
int n=825;
int x;
int main(){
	for(p=w;++p-w>>11?p=w,x=n--:1;)n?x=x%n*10+*p,*p=x/n:printf("%d",*p);
}
