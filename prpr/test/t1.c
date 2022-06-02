#ifdef KANAPO
	NO
	#ifndef TEST
		NO NO
	#endif
#else
	KOKO
	#ifdef K
		NO NO NO
	#endif
	#define K
	#ifdef K
		OK
	#else
		NO
	#endif
	#ifndef K
		PI
	#else
		PIPIPI
	#endif
#endif
