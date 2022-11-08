void perror ( char * s ) ;
int printf ( char * fmt , ... ) ;
int fprintf ( void * fp , char * fmt , ... ) ;
void * fopen ( char * path , char * mode ) ;
int sprintf ( char * str , char * fmt , ... ) ;
int scanf ( char * fmt , ... ) ;

int putchar(int c);

void *memset(void *b, int c, int len);

typedef struct va_list 
{
  int gp_offset ;
  int fp_offset ;
  void * overflow_arg_area ;
  void * reg_save_area ;
  
}
[ 1] va_list ;
void va_start ( va_list ap , ... ) ;
void vfprintf ( void * stream , char * format , va_list ap ) ;
extern void * __stdinp ;
extern void * __stdoutp ;
extern void * __stderrp ;
int k ;
float mysin ( float a ) ;
float mycos ( float b ) ;
int main ( ) 
{
  float A = 0;
  float B = 0;
  float i ;
  float j ;
  float z [ 1760] ;
  char b [ 1760] ;
  printf ( "\x1b[2J") ;
  for ( ;
  ;
  ) 
  {
    memset ( b , 32, 1760) ;
    memset ( z , 0, 7040) ;
    for ( j = 0;
    6.28> j ;
    j += 0.07) for ( i = 0;
    6.28> i ;
    i += 0.02) 
    {
      float c = mysin ( i ) ;
      float d = mycos ( j ) ;
      float e = mysin ( A ) ;
      float f = mysin ( j ) ;
      float g = mycos ( A ) ;
      float h = d + 2;
      float D = 1/ ( c * h * e + f * g + 5) ;
      float l = mycos ( i ) ;
      float m = mycos ( B ) ;
      float n = mysin ( B ) ;
      float t = c * h * g - f * e ;
      int x = 40+ 30* D * ( l * h * m - t * n ) ;
      int y = 12+ 15* D * ( l * h * n + t * m ) ;
      int o = x + 80* y ;
      int N = 8* ( ( f * e - c * d * g ) * m - c * d * e - f * g - l * d * n ) ;
      if ( 22> y && y > 0&& x > 0&& 80> x && D > z [ o ] ) 
      {
        z [ o ] = D ;
        b [ o ] = ".,-~:;=!*#$@"[ N > 0? N : 0] ;
        
      }
      
    }
    printf ( "\x1b[H") ;
    for ( k = 0;
    1761> k ;
    k ++ ) putchar ( k % 80? b [ k ] : 10) ;
    A += 0.04;
    B += 0.02;
    
  }
  
}
