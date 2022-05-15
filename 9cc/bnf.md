```bnf
filescope	= "struct" ident ( strct-block ";" | ptrs def-func)
			| typep ident (types ";" | def-func)

def-func	= "(" def-params ")" (; | stmt)
def-param	= typep ident
def-params	= def-param ("," def-params)? | def-param?

strct-block	= "{" struct-mems "}"
struct-mem	= def-var ";"
struct-mems	= struct-mem struct-mems? | struct-mem?

stmt		= (if-stmt | while-stmt | for-stmt | "{" stmts "}")
			| (def-var | "return" expr) ";"
stmts		= stmt stmts | stmt?

if-stmt		= "if" "(" expr ")" stmt ("else" stmt)?
while-stmt	= "while" "(" expr ")" stmt
for-stmt	= "for" "(" expr? ";" expr? ";" expr ")" stmt

def-var		= typep ident types

expr		= assign
assign		= equality ("=" assign)?
equality	= relational (("==" | "!=" ) relational)?
relational	= add (("<" | "<=" | ">" | ">=") add)?
add			= mul (("+" | "-") mul)?
mul			= unary (("*" | "/") unary)?
unary		= ("+" | "-") arrow
			| ("*" | "&") unary
			| "sizeof" unary
			| arrow

arrow		= primary (("->" | ".") ident)?*

primary		= ("(" expr ")"
			| ident ("(" call-params ")")?
			| strliteral
			| integer) derefs

call-params	= expr ("," call-params)? | expr?

deref		= "[" expr "]"
derefs		= deref derefs | deref?

typep		= ("struct" ident | ("int" | "char")) ptrs
types		= ("[" integer "]")?

ptr			= "*"
ptrs		= ptr ptrs? | ptr?

ident		= (symbol | alphabet) strs

integer		= number*

strliteral	= "\"" strs "\""
str			= alphabet | number | symbol
strs		= str strs | str?

alphabet	= [a-zA-Z]
number		= [1-9]
symbol		= "_"

escape		= "\\" ("a" | "b" | "f" | "n" | "r" | "t" | "v" | "0" | "\"")

``` 
