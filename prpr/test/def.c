# define KANAPO
# define TEST_1 100

# ifdef KANAPO
	OK
# endif

# ifndef KANAPO
	NO
#endif

# ifndef TEST_2
	#define TEST_2
#endif

# ifdef TEST_2
	OKOK
#endif

# ifndef TEST_2
	NONO
# endif  
