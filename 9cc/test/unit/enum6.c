int dint(int i);

typedef enum e1
{
	TK1,
	TK2,
	TK3,
	TK4
} Type;

int	main(void)
{
	Type	type;

	type = TK1;
	switch (type)
	{
		case TK1:	dint(0);	break;
		case TK2:	dint(1);	break;
		case TK3:	dint(2);	break;
		case TK4:	dint(3);	break;
	}

	type = TK2;
	switch (type)
	{
		case TK1:	dint(0);	break;
		case TK2:	dint(1);	break;
		case TK3:	dint(2);	break;
		case TK4:	dint(3);	break;
	}

	type = TK3;
	switch (type)
	{
		case TK1:	dint(0);	break;
		case TK2:	dint(1);	break;
		case TK3:	dint(2);	break;
		case TK4:	dint(3);	break;
	}

	type = TK4;
	switch (type)
	{
		case TK1:	dint(0);	break;
		case TK2:	dint(1);	break;
		case TK3:	dint(2);	break;
		case TK4:	dint(3);	break;
	}
}
