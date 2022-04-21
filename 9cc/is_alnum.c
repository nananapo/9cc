int is_uppercase(char c)
{
	return ('A' <= c && c <= 'Z');
}

int is_lowercase(char c)
{
	return ('a' <= c && c <= 'z');
}

int	is_number(char c)
{
	return ('0' <= c && c <= '9');
}

int	is_alphabet(char c)
{
	return is_uppercase(c) || is_lowercase(c);
}

int	can_use_beginning_of_var(char c)
{
	return is_alphabet(c) || (c == '_');
}

int	is_alnum(char c)
{
	return can_use_beginning_of_var(c) || is_number(c);
}
