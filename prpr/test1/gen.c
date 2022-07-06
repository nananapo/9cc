typedef enum e_token_kind 
{
  TK_IDENT , TK_NUM , TK_STR_LIT , TK_CHAR_LIT , TK_RESERVED , TK_EOD , TK_EOF 
}
TokenKind ;
typedef struct s_token 
{
  TokenKind kind ;
  char * str ;
  int len ;
  int is_directive ;
  int is_dq ;
  struct s_token * next ;
  
}
Token ;
typedef struct s_tokenize_env 
{
  char * str ;
  Token * token ;
  int can_define_dir ;
  
}
TokenizeEnv ;
typedef enum e_node 
{
  ND_INIT , ND_CODES , ND_INCLUDE , ND_DEFINE_MACRO , ND_UNDEF , ND_IFDEF , ND_IFNDEF 
}
NodeKind ;
typedef struct s_str_elem 
{
  char * str ;
  struct s_str_elem * next ;
  
}
StrElem ;
typedef struct s_node 
{
  NodeKind kind ;
  Token * codes ;
  int codes_len ;
  char * file_name ;
  int is_std_include ;
  char * macro_name ;
  StrElem * params ;
  struct s_node * stmt ;
  struct s_node * elif ;
  struct s_node * els ;
  struct s_node * next ;
  
}
Node ;
typedef struct s_parse_env 
{
  Token * token ;
  Node * node ;
  
}
ParseEnv ;
typedef struct s_macro 
{
  char * name ;
  StrElem * params ;
  Token * codes ;
  int codes_len ;
  struct s_macro * next ;
  
}
Macro ;
typedef struct s_gen_env 
{
  Macro * macros ;
  int print_count ;
  int nest_count ;
  char * stddir ;
  
}
GenEnv ;
void debug ( char * fmt , ... ) ;
char * read_file ( char * name ) ;
char * getdir ( char * full ) ;
int start_with ( char * haystack , char * needle ) ;
char * skip_space ( char * p ) ;
void error ( char * fmt , ... ) ;
void error_at ( char * at , char * fmt , ... ) ;
char * read_ident ( char * str ) ;
char * read_number ( char * str ) ;
int is_reserved_word ( char * str ) ;
int is_symbol ( char str ) ;
void add_str_elem ( StrElem * * list , char * str ) ;
Token * tokenize ( char * str ) ;
Node * parse ( Token * * tok , int nest ) ;
void gen ( Node * node ) ;
void set_currentdir ( char * stddir , char * filename ) ;
Node * create_node ( NodeKind kind ) ;
char * strlit_to_str ( char * str , int len ) ;
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
static GenEnv gen_env ;
static char * currentdir ;
static void add_macro ( char * name , StrElem * params , Token * codes , int codes_len ) 
{
  Macro * tmp ;
  tmp = malloc ( sizeof ( Macro ) ) ;
  tmp -> name = name ;
  tmp -> params = params ;
  tmp -> codes = codes ;
  tmp -> codes_len = codes_len ;
  tmp -> next = gen_env . macros ;
  gen_env . macros = tmp ;
  
}
static Macro * get_macro ( char * name ) 
{
  Macro * tmp ;
  tmp = gen_env . macros ;
  while ( tmp != 0) 
  {
    if ( strcmp ( tmp -> name , name ) == 0) return ( tmp ) ;
    tmp = tmp -> next ;
    
  }
  return ( 0) ;
  
}
static void apply_macro ( Macro * macro , Token * * tok ) 
{
  Node * node ;
  * tok = ( * tok ) -> next ;
  node = create_node ( ND_CODES ) ;
  node -> codes = macro -> codes ;
  node -> codes_len = macro -> codes_len ;
  gen ( node ) ;
  
}
static int consume_reserved ( Token * tok , char * str ) 
{
  int len ;
  len = strlen ( str ) ;
  if ( tok -> kind != TK_RESERVED || tok -> len != len || strncmp ( tok -> str , str , len ) != 0) return ( 0) ;
  return ( 1) ;
  
}
static void print_line ( void ) 
{
  int i ;
  printf ( "\n") ;
  i = - 1;
  while ( ++ i < gen_env . nest_count ) printf ( "  ") ;
  gen_env . print_count = 0;
  
}
static void codes ( Node * node ) 
{
  int i ;
  Token * code ;
  Macro * mactmp ;
  fprintf ( fopen ( "/dev/stderr", "a") , "# codes start %p\n", node -> codes ) ;
  code = node -> codes ;
  fprintf ( fopen ( "/dev/stderr", "a") , "# code check %p -> %p\n", node -> codes , code ) ;
  i = - 1;
  while ( ++ i < node -> codes_len ) 
  {
    fprintf ( fopen ( "/dev/stderr", "a") , "#  WHILE IN %d %p\n", i , code ) ;
    if ( code -> kind == TK_STR_LIT ) 
    {
      printf ( "\"%s\"", strndup ( code -> str , code -> len ) ) ;
      
    }
    else if ( code -> kind == TK_CHAR_LIT ) 
    {
      printf ( "'%s'", strndup ( code -> str , code -> len ) ) ;
      
    }
    else if ( code -> kind == TK_IDENT ) 
    {
      fprintf ( fopen ( "/dev/stderr", "a") , "#   IS IDENT %s\n", strndup ( code -> str , code -> len ) ) ;
      mactmp = get_macro ( strndup ( code -> str , code -> len ) ) ;
      if ( mactmp == 0) 
      {
        printf ( "%s ", strndup ( code -> str , code -> len ) ) ;
        
      }
      else 
      {
        apply_macro ( mactmp , & code ) ;
        continue ;
        
      }
      
    }
    else if ( code -> kind == TK_RESERVED ) 
    {
      if ( consume_reserved ( code , "{") ) 
      {
        print_line ( ) ;
        printf ( "{") ;
        gen_env . nest_count += 1;
        print_line ( ) ;
        
      }
      else if ( consume_reserved ( code , "}") ) 
      {
        gen_env . nest_count -= 1;
        print_line ( ) ;
        printf ( "}") ;
        print_line ( ) ;
        
      }
      else if ( consume_reserved ( code , ";") ) 
      {
        printf ( ";") ;
        print_line ( ) ;
        
      }
      else 
      {
        printf ( "%s ", strndup ( code -> str , code -> len ) ) ;
        
      }
      
    }
    else 
    {
      printf ( "%s", strndup ( code -> str , code -> len ) ) ;
      
    }
    code = code -> next ;
    
  }
  fprintf ( fopen ( "/dev/stderr", "a") , "# codes end\n") ;
  
}
static void load ( char * file_name ) 
{
  char * str ;
  Token * tok ;
  Node * node ;
  str = read_file ( file_name ) ;
  if ( str == 0) 
  {
    printf ( "%s\n", file_name ) ;
    error ( "^ ファイルが見つかりませんでした") ;
    
  }
  tok = tokenize ( str ) ;
  node = parse ( & tok , 0) ;
  gen ( node ) ;
  
}
static void include ( Node * node ) 
{
  char * file_name ;
  char str [ 10000] ;
  file_name = strlit_to_str ( node -> file_name , strlen ( node -> file_name ) ) ;
  if ( node -> is_std_include ) 
  {
    str [ 0] = '\0';
    strcat ( str , gen_env . stddir ) ;
    strcat ( str , file_name ) ;
    load ( str ) ;
    
  }
  else 
  {
    load ( file_name ) ;
    
  }
  
}
static void define_macro ( Node * node ) 
{
  add_macro ( node -> macro_name , node -> params , node -> codes , node -> codes_len ) ;
  
}
static void undef_macro ( Node * node ) 
{
  Macro * tmp ;
  Macro * last ;
  last = 0;
  tmp = gen_env . macros ;
  while ( tmp != 0) 
  {
    if ( strlen ( node -> macro_name ) != strlen ( tmp -> name ) || strncmp ( node -> macro_name , tmp -> name , strlen ( node -> macro_name ) ) != 0) 
    {
      last = tmp ;
      tmp = tmp -> next ;
      continue ;
      
    }
    if ( last == 0) gen_env . macros = tmp -> next ;
    else last -> next = tmp -> next ;
    break ;
    
  }
  
}
static void ifdef ( Node * node , int is_ifdef ) 
{
  Macro * macro ;
  macro = get_macro ( node -> macro_name ) ;
  if ( ( macro != 0) != is_ifdef ) 
  {
    if ( node -> els != 0) gen ( node -> els ) ;
    return ;
    
  }
  gen ( node -> stmt ) ;
  
}
void gen ( Node * node ) 
{
  fprintf ( fopen ( "/dev/stderr", "a") , "# gen\n") ;
  while ( node ) 
  {
    switch ( node -> kind ) 
    {
      case ND_INIT : fprintf ( fopen ( "/dev/stderr", "a") , "# init\n") ;
      break ;
      case ND_CODES : fprintf ( fopen ( "/dev/stderr", "a") , "# codes\n") ;
      codes ( node ) ;
      break ;
      case ND_INCLUDE : fprintf ( fopen ( "/dev/stderr", "a") , "# include\n") ;
      include ( node ) ;
      break ;
      case ND_DEFINE_MACRO : fprintf ( fopen ( "/dev/stderr", "a") , "# define macro\n") ;
      define_macro ( node ) ;
      break ;
      case ND_UNDEF : fprintf ( fopen ( "/dev/stderr", "a") , "# undef macro\n") ;
      undef_macro ( node ) ;
      break ;
      case ND_IFDEF : case ND_IFNDEF : fprintf ( fopen ( "/dev/stderr", "a") , "# ifdef ifndef\n") ;
      ifdef ( node , node -> kind == ND_IFDEF ) ;
      break ;
      
    }
    node = node -> next ;
    
  }
  
}
void set_currentdir ( char * stddir , char * filename ) 
{
  gen_env . stddir = stddir ;
  currentdir = getdir ( filename ) ;
  
}
