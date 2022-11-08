float mysin ( float a ) ;
float mycos ( float b ) ;

void putf(float a);

int main ( ) 
{
  float j ;

  for ( j = 0; 6.28> j ; j += 0.07)
  {
      float d = mycos ( j ) ;
      float f = mysin ( j ) ;
	  putf(d);
	  putf(f);
  } 
}
