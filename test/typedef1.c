typedef int Number;
typedef char *String;
typedef String *StringList;

Number	dint(Number a);
int		my_print(String str);

Number main(void)
{
	Number		a;
	String		s;
	StringList	ss;

	a = 100;
	s = "HelloWorld";
	dint(a);
	my_print(s);
	dint(sizeof(a));
	dint(sizeof(s));
	dint(sizeof(ss));
}
