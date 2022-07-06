int open ( char * pathname , int flags ) ;
int close ( int fildes ) ;
int read ( int fd , char * buf , int count ) ;
void perror ( char * s ) ;
void printf ( char * fmt , ... ) ;
void fprintf ( void * fp , char * fmt , ... ) ;
void * fopen ( char * path , char * mode ) ;
void * malloc ( int size ) ;
void * calloc ( int count , int size ) ;
void free ( void * ptr ) ;
void * memcpy ( void * s1 , void * s2 , int n ) ;
void * memmove ( void * s1 , void * s2 , int n ) ;
char * strcpy ( char * s1 , char * s2 ) ;
char * strncpy ( char * s1 , char * s2 , int n ) ;
char * strcat ( char * s1 , char * s2 ) ;
char * strncat ( char * s1 , char * s2 , int n ) ;
int memcmp ( void * s1 , void * s2 , int n ) ;
int strcmp ( char * s1 , char * s2 ) ;
int strcoll ( char * s1 , char * s2 ) ;
int strncmp ( char * s1 , char * s2 , int n ) ;
int strxfrm ( char * s1 , char * s2 , int n ) ;
void * memchr ( void * s , int c , int n ) ;
char * strchr ( char * s , int c ) ;
int strcspn ( char * s1 , char * s2 ) ;
char * strpbrk ( char * s1 , char * s2 ) ;
char * strrchr ( char * s , int c ) ;
int strspn ( char * s1 , char * s2 ) ;
char * strstr ( char * s1 , char * s2 ) ;
char * strtok ( char * s1 , char * s2 ) ;
char * memset ( void * s , int c , int n ) ;
char * strerror ( int errnum ) ;
int strlen ( char * s ) ;
char * strdup ( char * s1 ) ;
char * strndup ( char * s1 , int n ) ;
static char * read_all ( int fd ) ;
char * read_file ( char * name ) 
{
  int fd ;
  char * p ;
  if ( strcmp ( name , "-") == 0) 
  {
    fd = 0;
    
  }
  else 
  {
    fd = open ( name , 0) ;
    if ( fd == - 1) 
    {
      perror ( "error") ;
      return ( 0) ;
      
    }
    
  }
  p = read_all ( fd ) ;
  close ( fd ) ;
  return ( p ) ;
  
}
static char * repl ( char * source , int source_size , int dest_size ) 
{
  char * tmp ;
  int i ;
  tmp = calloc ( dest_size , sizeof ( char ) ) ;
  i = - 1;
  while ( ++ i < source_size ) tmp [ i ] = source [ i ] ;
  free ( source ) ;
  return ( tmp ) ;
  
}
static void mycopy ( char * dest , int index , char * source , int source_size ) 
{
  int i ;
  i = - 1;
  while ( ++ i < source_size ) dest [ index + i ] = source [ i ] ;
  
}
static char * read_all ( int fd ) 
{
  char * p ;
  int size ;
  char buf [ 1024] ;
  int result ;
  int index ;
  size = 1024;
  p = calloc ( size , sizeof ( char ) ) ;
  index = 0;
  while ( 1) 
  {
    result = read ( fd , buf , 1024) ;
    if ( result == 0) return ( p ) ;
    if ( result == - 1) return ( 0) ;
    if ( index + result >= size ) 
    {
      p = repl ( p , size , size * 2) ;
      size *= 2;
      
    }
    mycopy ( p , index , buf , result ) ;
    index += result ;
    
  }
  return ( p ) ;
  
}
char * getdir ( char * full ) 
{
  char * end ;
  end = strrchr ( full , '/') ;
  if ( end == 0) return ( "./") ;
  return ( strndup ( full , end - full ) ) ;
  
}
