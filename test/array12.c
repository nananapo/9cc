int main(){int a[3][3]; **a=1;*(*a+1)=2;*(*a+2)=3; **(a+1)=4;*(*(a+1)+1)=5;*(*(a+1)+2)=6; **(a+2)=7;*(*(a+2)+1)=8;*(*(a+2)+2)=9; return *(*(a+1)+2);}
