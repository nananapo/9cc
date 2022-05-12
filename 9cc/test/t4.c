int pint(int a);
int pspace(int a);
int pline();
int pcheck();

int main(){
 int a[3][3];
 int i;
 int j;
 for (i=0; i<3; i=i+1)
 {
  pcheck();
  for (j=0; j<3; j=j+1)
  {
   pcheck();
   a[i][j] = i * 3 + (j+1);
   pcheck();
  }
 }
 return a[0][0];
} 
