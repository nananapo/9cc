int main(){ int a[3][3]; int i; int j; for (i=0; i<3; i=i+1) for (j=0; j<3; j=j+1) a[i][j] = i * 3 + (j+1); return a[1][2];}
