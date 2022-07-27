#include "9cc.h"
#include "stack.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

int	max(int a, int b);
int	min(int a, int b);
int	align_to(int n, int align);
static void	push(void);
static void	pushi(int data);
static void	pop(char *reg);
static void	mov(char *dst, char *from);
static void	movi(char *dst, int i);
static void	cmps(char *dst, char *from);
static void	cmp(Type *dst, Type *from);
char	*get_str_literal_name(int index);
static void	comment(char *c);
static void	store_value(int size);
static void	store_ptr(int size, bool minus_step);
static void	load(Type *type);
static void	lval(Node *node);
static void	call(Node *node);
static void	cast(Type *from, Type *to);
static void	arrow(Node *node, bool as_addr);
static void	add_check_pointer(Type *restype, Node **lhs, Node **rhs, bool can_replace);
static void	create_add(bool is_add, Type *l, Type *r);
static void	load_lval_addr(Node *node);
static void	funcdef(Node *node);
static void	print_global_constant(Node *node, Type *type);
static void	globaldef(Node *node);
void	gen(Node *node);

extern t_str_elem	*str_literals;
extern int			switchCaseCount;
extern Stack		*sbstack;

int					jumpLabelCount = 0;
char				*arg_regs[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
int					stack_count = 0;

static Node	*g_func_now;

int	max(int a, int b)
{
	if (a > b)
		return (a);
	return (b);
}

int	min(int a, int b)
{
	if (a < b)
		return (a);
	return (b);
}

int	align_to(int n, int align)
{
	if (align == 0)
		return (n);
	return ((n + align - 1) / align * align);
}

static void	push()
{
	stack_count += 8;
	printf("    %s %s # stack= %d -> %d\n", ASM_PUSH, RAX, stack_count - 8, stack_count);
}

static void	pushi(int data)
{
	stack_count += 8;
	printf("    %s %d # stack= %d -> %d\n", ASM_PUSH, data, stack_count - 8, stack_count);
}

static void	pop(char *reg)
{
	stack_count -= 8;
	printf("    pop %s # stack= %d -> %d\n", reg, stack_count + 8, stack_count);
}

static void	mov(char *dst, char *from)
{
	printf("    %s %s, %s\n", ASM_MOV, dst, from);
}

static void	movi(char *dst, int i)
{
	printf("    %s %s, %d\n", ASM_MOV, dst, i);
}

static void	cmps(char *dst, char *from)
{
	printf("    cmp %s, %s\n", dst, from);
}

// TODO 構造体の比較
// rax, rdi
static void	cmp(Type *dst, Type *from)
{
	if (type_equal(dst, from))
	{
		if (dst->ty == TY_PTR || dst->ty == TY_ARRAY)
			cmps(RAX, RDI);
		else if (dst->ty == TY_CHAR || dst->ty == TY_BOOL)
			cmps(AL, DIL);
		else if (dst->ty == TY_INT || dst->ty == TY_ENUM)
			cmps(EAX, EDI);
		return ;
	}
	cmps(RAX, RDI);
}

char	*get_str_literal_name(int index)
{

	char	*tmp;

	tmp = calloc(100, sizeof(char));
	sprintf(tmp, "L_STR_%d", index);
	return (tmp);
}

static void	comment(char *c)
{
	printf("# %s\n", c);
}

// RAXからR10(アドレス)に値をストアする
static void	store_value(int size)
{
	if (size == 1)
		mov("byte ptr [r10]", AL);
	else if (size == 2)
		mov("word ptr [r10]", EAX);
	else if (size == 4)
		mov("dword ptr [r10]", EAX);
	else if (size == 8)
		mov("[r10]", RAX);
	else
		error("store_valueに不正なサイズ(%d)の値を渡しています", size);
}

// アドレス(RAX)の先の値をR10(アドレス)にストアする
static void store_ptr(int size, bool minus_step)
{
	int		delta;
	char	*dest;
	char	*from;
	char	*op;
	int		i;

	op = "+";
	if (minus_step)
		op = "-";

	dest = R10;
	
	for (i = 0; i < size; i += 8)
	{
		delta = size - i;
		if (delta >= 8)
		{
			from = R11;
			if (i != 0)
			{
				printf("    %s %s, [%s + %d]\n", ASM_MOV, from, RAX, i);
				printf("    %s [%s %s %d], %s\n", ASM_MOV, dest, op, i, from);
			}
			else
			{
				printf("    %s %s, [%s]\n", ASM_MOV, from, RAX);
				printf("    %s [%s], %s\n", ASM_MOV, dest, from);
			}
			continue ;
		}
		if (delta >= 4)
		{
			from = R11D;
			if (i != 0)
			{
				printf("    %s %s, %s [%s + %d]\n", ASM_MOV, from, DWORD_PTR, RAX, i);
				printf("    %s %s [%s %s %d], %s\n", ASM_MOV, DWORD_PTR, dest, op,  i, from);
			}
			else
			{
				printf("    %s %s, %s [%s]\n", ASM_MOV, from, DWORD_PTR, RAX);
				printf("    %s %s [%s], %s\n", ASM_MOV, DWORD_PTR, dest, from);
			}
			i += 4;
			delta -= 4;
		}
		if (delta >= 2)
		{
			from = R11W;
			if (i != 0)
			{
				printf("    %s %s, %s [%s + %d]\n", ASM_MOV, from, WORD_PTR, RAX, i);
				printf("    %s %s [%s %s %d], %s\n", ASM_MOV, WORD_PTR, dest, op, i, from);
			}
			else
			{
				printf("    %s %s, %s [%s]\n", ASM_MOV, from, WORD_PTR, RAX);
				printf("    %s %s [%s], %s\n", ASM_MOV, WORD_PTR, dest, from);
			}
			i += 2;
			delta -= 2;
		}
		if (delta >= 1)
		{
			from = R11B;
			if (i != 0)
			{
				printf("    %s %s, %s [%s + %d]\n", ASM_MOV, from, BYTE_PTR, RAX, i);
				printf("    %s %s [%s %s %d], %s\n", ASM_MOV, BYTE_PTR, dest, op, i, from);
			}
			else
			{
				printf("    %s %s, %s [%s]\n", ASM_MOV, from, BYTE_PTR, RAX);
				printf("    %s %s [%s], %s\n", ASM_MOV, BYTE_PTR, dest, from);
			}
			i += 1;
			delta -= 1;
		}
	}
}

// raxをraxに読み込む
static void	load(Type *type)
{
	
	if (type->ty == TY_PTR)
	{
		mov(RAX, "[rax]");
		return ;
	}
	else if (type->ty == TY_CHAR || type->ty == TY_BOOL)
	{
		printf("    mov al, %s [%s]\n", BYTE_PTR, RAX);
		printf("    movzx %s, %s\n", RAX, AL);
		return ;
	}
	else if (type->ty == TY_INT || type->ty == TY_ENUM)
	{
		printf("    mov %s, %s [%s]\n", EAX, DWORD_PTR, RAX);
		//printf("    movzx %s, %s\n", RAX, EAX);
		return ;
	}
	else if (type->ty == TY_ARRAY)
	{
		return ;
	}
	else if (type->ty == TY_STRUCT || type->ty == TY_UNION)
	{
		// TODO とりあえず8byteまで
		// mov(RAX, "[rax]");
		return ;
	}
}

// 変数のアドレスをraxに移動する
static void	lval(Node *node)
{
	int	offset;

	if (node->kind != ND_LVAR && node->kind != ND_LVAR_GLOBAL)
		error("変数ではありません Kind:%d Type:%d", node->kind, node->type->ty);

	if (node->kind == ND_LVAR)
	{
		offset = node->lvar->offset;
		mov(RAX, RBP);
		if (offset > 0)
			printf("    sub %s, %d\n", RAX, offset);
		else if (offset < 0)
			printf("    add %s, %d\n", RAX, - offset);
	}
	else if (node->kind == ND_LVAR_GLOBAL)
	{
		printf("    mov rax, [rip + _%s@GOTPCREL]\n", strndup(node->var_name, node->var_name_len));
	}
}

static void	call(Node *node)
{
	int		size;
	int 	rbp_offset;
	int		arg_count = 0;
	int		push_count = 0;
	bool	is_aligned;
	Node	*tmp;
	int		i;
	int		j;
	int		pop_count;

	LVar	*lvtmp;
	int		min_offset;
	int		max_argregindex;

	// va_startマクロ
	if (node->flen == 8
	&& strncmp(node->fname, "va_start", 8) == 0)
	{
		// register save
		printf("    sub rsp, 48\n");
		printf("    mov [rsp + 0], rdi\n");
		printf("    mov [rsp + 8], rsi\n");
		printf("    mov [rsp + 16], rdx\n");
		printf("    mov [rsp + 24], rcx\n");
		printf("    mov [rsp + 32], r8\n");
		printf("    mov [rsp + 40], r9\n");

		// overflow arg area
		min_offset = 0;
		max_argregindex = -1;
		for (lvtmp = g_func_now->locals; lvtmp; lvtmp = lvtmp->next)
		{
			min_offset = min(lvtmp->offset, min_offset);
			max_argregindex = max(max_argregindex, lvtmp->arg_regindex);
			printf(" # LOOP %d\n", lvtmp->arg_regindex);
		}

		// 一つめの引数を読み込み
		if (node->args == NULL)
			error("va_startに引数がありません");

		lval(node->args);
		mov(RDI, RAX);

		printf("    mov dword ptr [rdi], %d\n", (1 + max_argregindex) * 8); // gp offset
		printf("    mov dword ptr [rdi + 4], 0\n\n"); // fp_offset

		printf("    mov rax, rbp\n");
		printf("    add rax, %d\n", - min_offset);
		printf("    mov [rdi + 8], rax\n\n"); // overflow arg area

		printf("    mov [rdi + 16], rsp\n\n"); // reg save area

		printf("    # VA_START\n");
		printf("    # gp_offset : %d\n", (1 + max_argregindex) * 8);
		printf("    # fp_offset : %d\n", 0 * 8);

		return ;
	}

	for (tmp = node->args; tmp; tmp = tmp->next)
	{
		arg_count++;

		size = get_type_size(type_array_to_ptr(tmp->locals->type));

		printf("# PUSH ARG %s (%d)\n",
			strndup(tmp->locals->name, tmp->locals->len),
			size);

		gen(tmp);

		if (tmp->locals->arg_regindex != -1)
		{
			if (size <= 8)
			{
				push();
				push_count++;
				continue ;
			}
			// size > 8なものは必ずstructであると願います( ;∀;)
			// RAXにアドレスが入っていると想定
			mov(RDI, RAX);
			for (i = 0; i < size; i += 8)
			{
				if (i == 0)
					printf("    %s %s, [%s]\n",
						ASM_MOV, RAX, RDI);
				else
					printf("    %s %s, [%s + %d]\n",
						ASM_MOV, RAX, RDI, i);
				push();
				push_count++;
			}
		}
		else
		{
			push();
			push_count++;
		}
	}

	// 16 byteアラインチェック
	rbp_offset = 0;
	for (tmp = node->args; tmp; tmp = tmp->next)
	{
		if (tmp->locals->arg_regindex != -1)
			continue ;
		rbp_offset = min(rbp_offset,
						tmp->locals->offset - align_to(get_type_size(tmp->locals->type), 8) + 8);
	}

	// マイナスなのでプラスにする
	rbp_offset = - rbp_offset;

	is_aligned = (stack_count + rbp_offset + 8) % 16 == 0;
	if (!is_aligned)
		rbp_offset += 8;

	printf("# RBP_OFFSET %d (is_aligned : %d)\n", rbp_offset, is_aligned);

	pop_count = 0;
	// 後ろから格納していく
	for (i = arg_count; i > 0; i--)
	{
		// i番目のtmpを求める
		tmp = node->args;
		for (j = 1; j < i; j++)
			tmp = tmp->next;

		printf("# POP %s\n", strndup(tmp->locals->name, tmp->locals->len));
	
		size = get_type_size(type_array_to_ptr(tmp->locals->type));

		// レジスタに入れる
		if (tmp->locals->arg_regindex != -1)
		{
			if (size <= 8)
			{
				printf("    %s %s, [%s + %d]\n", ASM_MOV, RAX, RSP, pop_count++ * 8);
				mov(arg_regs[tmp->locals->arg_regindex], RAX);
				continue ;
			}
			// size > 8なものは必ずstructであると願います( ;∀;)
			// RAXにアドレスが入っていると想定
			for (j = size - 8; j >= 0; j -= 8)
			{
				printf("    %s %s, [%s + %d]\n", ASM_MOV,
					arg_regs[tmp->locals->arg_regindex - j / 8],
					 RSP, pop_count++ * 8);
			}
			continue ;
		}

		printf("# OFFSET %d\n", tmp->locals->offset);

		// スタックに積む
		// 必ず8byteアラインなので楽々実装
		size = align_to(size, 8);

		printf("    %s %s, [%s + %d]\n", ASM_MOV, RAX, RSP, pop_count++ * 8);
		mov(R10, RSP);

		printf("    sub %s, %d\n", R10,
				(tmp->locals->offset + 16) + rbp_offset);

		// ptr先を渡すのはSTRUCTだけ
		if (tmp->type->ty == TY_STRUCT
			|| tmp->type->ty == TY_UNION)
			store_ptr(size, false);
		else
			store_value(size);
	}

	// rspを移動する
	if (rbp_offset != 0)
	{
		if (!is_aligned)
			comment("aligned + 8");
		printf("    sub rsp, %d # rbp_offset\n", rbp_offset);
	}

	// 返り値がMEMORYなら、返り値の格納先のアドレスをRDIに設定する
	if (is_memory_type(node->type))
	{
		printf("    lea rdi, [rbp - %d]\n", node->call_mem_stack->offset);
	}

	// call
	printf("# CALL RBP_OFFSET: %d\n", rbp_offset);
	if (node->is_variable_argument)
		printf("    mov al, 0\n");
	printf("    call _%s\n", strndup(node->fname, node->flen));

	// rspを元に戻す
	if (rbp_offset != 0)
	{
		printf("    add rsp, %d # rbp_offset\n", rbp_offset);
	}

	// stack_countをあわせる
	printf("    add rsp, %d # pop_count\n", pop_count * 8);
	printf("# POP ALL %d -> %d\n", stack_count, stack_count - pop_count * 8);
	stack_count -= pop_count * 8;

	// 返り値がMEMORYなら、raxにアドレスを入れる
	if (is_memory_type(node->type))
	{
		printf("    lea rax, [rbp - %d]\n", node->call_mem_stack->offset);
	}
	// 返り値がstructなら、rax, rdxを移動する
	else if (node->type->ty == TY_STRUCT)
	{
		size = align_to(get_type_size(node->type), 8);
		printf("    lea %s, [rbp - %d]\n", RDI, node->call_mem_stack->offset);
		if (size > 0)
			printf("    mov [%s], %s\n", RDI, RAX);
		if (size > 8)
			printf("    mov [%s + 8], %s\n", RDI, RDX);
		mov(RAX, RDI);
	}
	return;
}

// raxに入っている型fromをtoに変換する
static void	cast(Type *from, Type *to)
{
	int		size1;
	int		size2;
	char	*name1;
	char	*name2;

	// 型が同じなら何もしない
	if (type_equal(from, to))
		return ;

	name1 = get_type_name(from);
	name2 = get_type_name(to);
	printf("# cast %s -> %s\n", name1, name2);

	// ポインタからポインタのキャストは何もしない
	if (is_pointer_type(from)
	&& is_pointer_type(to))
		return ;
	
	size1 = get_type_size(from);
	size2 = get_type_size(to);

	// ポインタから整数へのキャストは情報を落とす
	if (is_pointer_type(from)
	&& is_integer_type(to))
	{
		pushi(0);
		mov(R10, RSP);
		store_value(size2);
		pop(RAX);
		return ;
	}

	// 整数からポインタは0埋めで拡張
	// (ポインタはunsinged long long intなので)
	if (is_integer_type(from)
	&& is_pointer_type(to))
	{
		pushi(0);
		mov(R10, RSP);
		store_value(size1);
		pop(RAX);
		return ;
	}

	// 整数から整数は符号を考えながらキャスト
	if (is_integer_type(from)
	&& is_integer_type(to))
	{
		// TODO unsigned
		// 符号拡張する
		if (size1 < size2)
		{
			if (size1 == 1)
			{
				printf("    movsx %s, %s # cast %d -> %d\n", RAX, AL, size1, size2);
			}
			else if (size1 == 4)
			{
				printf("    movsx %s, %s # cast %d -> %d\n", RAX, EAX, size1, size2);
			}
			else
				error("8byte -> 8byteのキャストは無い");
		}
		return ;
	}

	if (from != NULL)
		fprintf(stderr, "from : %d\n", from->ty);
	if (to != NULL)
		fprintf(stderr, "to : %d\n", to->ty);
		
	error("%sから%sへのキャストが定義されていません\n (addr %p, %p)", name1, name2, from, to);
}

static void	arrow(Node *node, bool as_addr)
{
	int offset;

	// arrowかその他の可能性がある
	if (node->lhs->kind == ND_MEMBER_VALUE
	|| node->lhs->kind == ND_MEMBER_PTR_VALUE)
		arrow(node->lhs, node->kind == ND_MEMBER_VALUE);
	else
		gen(node->lhs);

	// offsetを足す
	offset = node->elem->offset;
	printf("    add rax, %d # offset\n", offset);

	// 値として欲しいなら値にする
	if (!as_addr)
		load(node->elem->type);
}

static void	mul_prologue(Node *node)
{
	gen(node->rhs);
	push();
	gen(node->lhs);
	pop("rdi");
}

static void	add_check_pointer(Type *restype, Node **lhs, Node **rhs, bool can_replace)
{
	// 結果がポインタ型なら
	if (restype->ty != TY_PTR && restype->ty != TY_ARRAY)
		return ;

	// 左辺をポインタ型にする
	if (can_replace &&
		((*rhs)->type->ty == TY_PTR || (*rhs)->type->ty == TY_ARRAY))
	{
		Node *tmp = *lhs;
		*lhs = *rhs;
		*rhs = tmp;
	}

	// 右辺が整数型なら掛け算に置き換える
	if (is_integer_type((*rhs)->type))
	{
		// ポインタの先のサイズを掛ける
		*rhs = new_node(ND_MUL, *rhs,
						new_node_num(get_type_size((*lhs)->type->ptr_to)));
		(*rhs)->type = new_primitive_type(TY_INT);
	}
}

// rax, rdi
static void	create_add(bool is_add, Type *l, Type *r)
{
	int	size;

	printf("    # add(%d) %s(%d) + %s(%d)\n", is_add,
			get_type_name(l), get_type_size(l),
			get_type_name(r), get_type_size(r));

	// とりあえずinteger以外は無視
	if (is_integer_type(l))
	{
		size = get_type_size(l);
		if (size == 4)
			printf("    movsxd rax, eax # sizeup\n");
		else if (size == 2)
			printf("    movsx rax, ax # sizeup\n");
		else if (size == 1)
			printf("    movsx rax, al # sizeup\n");
	}

	if (is_integer_type(r))
	{
		size = get_type_size(r);
		if (size == 4)
			printf("    movsxd rdi, edi # sizeup\n");
		else if (size == 2)
			printf("    movsx rdi, di # sizeup\n");
		else if (size == 1)
			printf("    movsx rdi, dil # sizeup\n");
	}

	if (is_add)
		printf("    add rax, rdi\n");
	else
		printf("    sub rax, rdi\n");
}

static void	relational_prologue(Node *node)
{
	gen(node->rhs);
	push();
	gen(node->lhs);
	pop("rdi");
}

static void	equality_prologue(Node *node)
{
	gen(node->rhs);
	push();
	gen(node->lhs);
	pop("rdi");
}

static void	load_lval_addr(Node *node)
{
	if (node->kind == ND_LVAR)
		lval(node);
	else if (node->kind == ND_LVAR_GLOBAL)
		lval(node);
	else if (node->kind == ND_DEREF)
		gen(node->lhs);// ここもDEREFと同じようにやってる！！！！！
	else if (node->kind == ND_MEMBER_VALUE)
		arrow(node, true);
	else if (node->kind == ND_MEMBER_PTR_VALUE)
		arrow(node, true);
	else
		error("左辺値が識別子かアドレスではありません");
}

static void	assign_prologue(Node *node)
{
	printf("#ASSIGN %d\n", node->lhs->kind);
	load_lval_addr(node->lhs);	
	push();
}

static void	assign_epilogue(Type *type)
{
	pop("r10");

	// storeする
	if (type->ty == TY_ARRAY)
		// TODO これOKなの？
		// ARRAYに対する代入がうまくいかない気がする
		store_value(8);
	else if(type->ty == TY_STRUCT
			|| type->ty == TY_UNION)
		store_ptr(get_type_size(type), false);
	else
		store_value(get_type_size(type));
}

static void	funcdef(Node *node)
{
	char	*funcname;
	LVar	*lvtmp;
	int		stack_size;
	int		maxoff;
	int		index;
	int		size;

	funcname = strndup(node->fname, node->flen);
	stack_count = 0;
	g_func_now = node;

	printf(".section	__TEXT,__text,regular,pure_instructions\n");
	if (!node->is_static)
		printf(".globl _%s\n", funcname);
	printf("_%s:\n", funcname);	

	// prologue
	mov(RAX, RBP);
	push();
	mov(RBP, RSP);

	if (node->locals != NULL)
	{
		maxoff = 0;
		for (lvtmp = node->locals; lvtmp; lvtmp = lvtmp->next)
		{
			printf("# VAR %s %d\n", strndup(lvtmp->name, lvtmp->len), lvtmp->offset);
			maxoff = max(maxoff, lvtmp->offset);	
		}
		stack_size = align_to(maxoff, 8);
	}
	else
		stack_size = 0;

	stack_count += stack_size; // stack_sizeを初期化

	printf("# STACKSIZE : %d\n", stack_size);

	if (stack_size != 0)
	{
		printf("    sub rsp, %d\n", stack_size);// stack_size

		for (lvtmp = node->locals; lvtmp; lvtmp = lvtmp->next)
		{
			if (!lvtmp->is_arg)
				continue ;
			printf("# ARG %s\n", strndup(lvtmp->name, lvtmp->len));
			if (lvtmp->arg_regindex != -1)
			{
				index = lvtmp->arg_regindex;
				size = align_to(get_type_size(lvtmp->type), 8);

				mov(R10, RBP);
				printf("    sub %s, %d\n", R10, lvtmp->offset);

				// とりあえず、かならず8byte境界になっている
				while (size >= 8)
				{
					mov(RAX, arg_regs[index--]);
					store_value(8);
					size -= 8;
					if (size != 0)
						printf("    add %s, 8\n", R10);
				}
			}
		}
		printf("# ARG_END\n");
	}

	// 返り値がMEMORYなら、rdiからアドレスを取り出す
	if (is_memory_type(node->ret_type))
	{
		printf("    mov [rbp - %d], %s\n", node->locals->offset, RDI);
	}

	gen(node->lhs);

	// TODO 返り値の型がMEMORYのとき、最後の式がreturnじゃないとセグフォします

	// epilogue
	printf(".Lepi_%s:\n", funcname);

	// MEMORYなら、rdiのアドレスに格納
	if (is_memory_type(node->ret_type))
	{
		printf("    mov %s, [rbp - %d]\n", R10, node->locals->offset);
		store_ptr(get_type_size(node->ret_type), false);

		// RDIを復元する
		printf("    mov %s, [rbp - %d]\n", RDI, node->locals->offset);
	}
	// STRUCTなら、rax, rdxに格納
	else if (node->ret_type->ty == TY_STRUCT)
	{
		size = align_to(get_type_size(node->ret_type), 8);
		if (size > 8)
			printf("    mov %s, [%s  - 8]\n", RDX, RAX);
		if (size > 0)
			printf("    mov %s, [%s]\n", RAX, RAX);
	}

	mov(RSP, RBP);
	pop(RBP);
	printf("    ret\n");

	stack_count -= stack_size;
	printf("#count %d\n", stack_count);
	
	if (stack_count != 0)
		error("stack_countが0ではありません");

	g_func_now = NULL;
}

static void	print_global_constant(Node *node, Type *type)
{
	Node	*notmp;

	if (type_equal(type, new_primitive_type(TY_INT)))
	{
		printf("    .long %d\n", node->val);
	}
	else if (type_equal(type, new_primitive_type(TY_CHAR)))
	{
		printf("    .byte %d\n", node->val);
	}
 	else if (type_equal(type, new_type_ptr_to(new_primitive_type(TY_CHAR)))
			&& node->str_index >= 0)
	{
		// TODO arrayのchar
		printf("    .quad %s\n",
				get_str_literal_name(node->str_index));
	}
	else if (is_pointer_type(type))
	{
		// TODO 数のチェックはしてない
		// array array
		for (notmp = node; notmp; notmp = notmp->next)
			print_global_constant(notmp, type->ptr_to);
	}
	else
		error("print_global_constant : 未対応の型 %s", get_type_name(type));
}

static void globaldef(Node *node)
{
	char	*name;

	if (node->is_extern)
		return ;
	name = strndup(node->var_name, node->var_name_len);
	if (!node->is_static)
		printf(".globl _%s\n", name);
	if (node->global_assign == NULL)
	{
		printf("    .comm _%s,%d,2\n",
				name, get_type_size(node->type));
	}
	else
	{
		printf(".section	__DATA, __data\n");
		printf("_%s:\n", name);
		print_global_constant(node->global_assign, node->type);
	}
}

void	gen(Node *node)
{
	int		lend;
	int		lbegin;
	int		lbegin2;
	int		lelse;
	SBData	*sbdata;

	if (node->kind == ND_PROTOTYPE
	|| node->kind == ND_DEFVAR
	|| node->kind == ND_STRUCT_DEF
	|| node->kind == ND_UNION_DEF
	|| node->kind == ND_TYPEDEF
	|| node->kind == ND_ENUM_DEF)
		return;

	switch(node->kind)
	{
		case ND_FUNCDEF:
		{
			funcdef(node);
			return;
		}
		case ND_DEFVAR_GLOBAL:
		{
			globaldef(node);
			return;
		}
		case ND_RETURN:
		{
			if (node->lhs != NULL)
				gen(node->lhs);
			// TODO 型チェックをしておく
			printf("    jmp .Lepi_%s\n", strndup(g_func_now->fname, g_func_now->flen));
			return;
		}
		case ND_IF:
		{
			// if
			gen(node->lhs);
			mov(RDI, "0");
			cmp(node->lhs->type, new_primitive_type(TY_INT));

			lend = jumpLabelCount++;

			if (node->elsif != NULL)
			{	
				lelse = jumpLabelCount++;
				printf("    je .Lelse%d\n", lelse);
				gen(node->rhs);
				printf("    jmp .Lend%d\n", lend);

				// else if
				printf(".Lelse%d:\n", lelse);
				gen(node->elsif);
			}
			else if (node->els == NULL)
			{
				printf("    je .Lend%d\n", lend);
				gen(node->rhs);
			}
			else
			{
				lelse = jumpLabelCount++;
				printf("    je .Lelse%d\n", lelse);
				gen(node->rhs);
				printf("    jmp .Lend%d\n", lend);

				// else
				printf(".Lelse%d:\n", lelse);
				gen(node->els);
			}
			printf(".Lend%d:\n", lend);
			return;
		}
		case ND_WHILE:
		{
			lbegin = jumpLabelCount++;
			lend = jumpLabelCount++;
			
			printf(".Lbegin%d:\n", lbegin); // continue先
			
			// if
			gen(node->lhs);
			mov(RDI, "0");
			cmp(node->lhs->type, new_primitive_type(TY_INT));
			printf("    je .Lend%d\n", lend);
			
			// while block
			sb_forwhile_start(lbegin, lend);
			if (node->rhs != NULL)
				gen(node->rhs);
			sb_end();
			
			// next
			printf("    jmp .Lbegin%d\n", lbegin);
			
			// end
			printf(".Lend%d:\n", lend); //break先
			return;
		}
		case ND_DOWHILE:
		{
			lbegin = jumpLabelCount++;
			lbegin2 = jumpLabelCount++;
			lend = jumpLabelCount++;
			
			printf(".Lbegin%d:\n", lbegin);

			// while block
			sb_forwhile_start(lbegin2, lend);
			if (node->lhs != NULL)
				gen(node->lhs);
			sb_end();

			// if
			printf(".Lbegin%d:\n", lbegin2); // continueで飛ぶ先
			gen(node->rhs);
			mov(RDI, "0");
			cmp(node->rhs->type, new_primitive_type(TY_INT));
			printf("    jne .Lbegin%d\n", lbegin);
			printf(".Lend%d:\n", lend); // break先
			return;
		}
		case ND_FOR:
		{
			lbegin = jumpLabelCount++;
			lbegin2 = jumpLabelCount++;
			lend = jumpLabelCount++;
			
			// init
			if (node->for_init != NULL)
				gen(node->for_init);

			printf(".Lbegin%d:\n", lbegin);
			
			// if
			if(node->for_if != NULL)
			{
				gen(node->for_if);
				mov(RDI, "0");
				cmp(node->for_if->type, new_primitive_type(TY_INT));
				printf("    je .Lend%d\n", lend);
			}

			// for-block
			sb_forwhile_start(lbegin2, lend);
			if (node->lhs != NULL)
				gen(node->lhs);
			sb_end();

			printf(".Lbegin%d:\n", lbegin2); // continue先
			// next
			if(node->for_next != NULL)
				gen(node->for_next);

			printf("    jmp .Lbegin%d\n", lbegin);
			
			//end
			printf(".Lend%d:\n", lend); // break先
			return;
		}
		case ND_SWITCH:
		{
			lbegin = switchCaseCount++;
			lend = jumpLabelCount++;

			// 評価
			gen(node->lhs);
			printf("    mov [rsp - 8], rax\n"); //結果を格納

			// if
			printf("    # switch def:%d, end:%d\n", lbegin, lend);
			SwitchCase	*sw_tmp;
			for (sw_tmp = node->switch_cases; sw_tmp; sw_tmp = sw_tmp->next)
			{
				printf("    mov rax, [rsp - 8]\n");
				movi(RDI, sw_tmp->value);
				cmp(node->lhs->type, node->lhs->type);
				printf("    je .Lswitch%d\n", sw_tmp->label);
			}
			// defaultかendに飛ばす
			if (node->switch_has_default)
				printf("    jmp .Lswitch%d\n", lbegin);
			else
				printf("    jmp .Lend%d\n", lend);

			printf("    # switch in\n");
			// 文を出力
			sb_switch_start(node->lhs->type, lend, lbegin);
			gen(node->rhs);
			sb_end();

			printf(".Lend%d:\n", lend);
			return ;
		}
		case ND_CASE:
		{
			printf(".Lswitch%d: # case %d\n", node->switch_label, node->val);
			return ;
		}
		case ND_BREAK:
		{
			sbdata = sb_peek();
			// 一応チェック
			if (sbdata == NULL)
				error("breakに対応する文が見つかりません");
			printf("jmp .Lend%d # break\n", sbdata->endlabel);
			return ;
		}
		case ND_CONTINUE:
		{
			sbdata = sb_search(false);
			// 一応チェック
			if (sbdata == NULL)
				error("continueに対応する文が見つかりません");
			printf("jmp .Lbegin%d # continue\n", sbdata->startlabel);
			return ;
		}
		case ND_DEFAULT:
		{
			sbdata = sb_search(true);
			// 一応チェック
			if (sbdata == NULL)
				error("defaultに対応する文が見つかりません");
			printf(".Lswitch%d: # default\n", sbdata->defaultLabel);
			return ;
		}
		case ND_BLOCK:
		{
			while(node != NULL)
			{
				if (node->lhs == NULL)
					return;
				gen(node->lhs);
				node = node->rhs;
			}
			return;
		}
		case ND_ASSIGN:
		{
			assign_prologue(node);
			gen(node->rhs);
			assign_epilogue(node->type);
			return ;
		}
		case ND_COMP_ADD:
		{
			assign_prologue(node);

			push();

			add_check_pointer(node->type, &node->lhs, &node->rhs, false);

			gen(node->rhs);
			pop(RDI);

			if (node->type->ty == TY_ARRAY)
				printf("    add rax, rdi\n");
			else
				printf("    add rax, [rdi]\n");

			assign_epilogue(node->type);
			return ;
		}
		case ND_COMP_SUB:
		{
			assign_prologue(node);

			push();
			add_check_pointer(node->type, &node->lhs, &node->rhs, false);

			gen(node->rhs);
			mov(RDI, RAX);
			pop(RAX);
			if (node->type->ty == TY_ARRAY)
				printf("    sub rax, rdi\n");
			else
			{
				printf("    mov rax, [rax]\n");
				printf("    sub rax, rdi\n");
			}

			assign_epilogue(node->type);
			return ;
		}
		case ND_COMP_MUL:
		{
			assign_prologue(node);

			push();
			gen(node->rhs);
			mov(RDI, RAX);
			pop(RAX);
			// TODO 整数だけ？
			printf("    mov rax, [rax]\n");
			printf("    imul rax, rdi\n");

			assign_epilogue(node->type);
			return;
		}
		case ND_COMP_DIV:
		{
			assign_prologue(node);

			push();
			gen(node->rhs);
			mov(RDI, RAX);
			pop(RAX);
			// TODO 整数だけ？
			printf("    mov rax, [rax]\n");
			printf("    cdq\n");
			printf("    idiv edi\n");

			assign_epilogue(node->type);
			return ;
		}
		case ND_COMP_MOD:
		{
			assign_prologue(node);

			push();
			gen(node->rhs);
			mov(RDI, RAX);
			pop(RAX);
			// TODO 整数だけ？
			printf("    mov rax, [rax]\n");
			printf("    cdq\n");
			printf("    idiv edi\n");
			mov(RAX, RDX);

			assign_epilogue(node->type);
			return ;
		}
		case ND_COND_AND:
		{
			lend = jumpLabelCount++;

			// 左辺を評価
			gen(node->lhs);
			mov(RDI, "0");
			cmp(node->lhs->type, new_primitive_type(TY_INT));
			printf("    setne al\n"); // 0と等しくないかをalに格納
			printf("    je .Lcond%d\n", lend); // 0ならスキップ

			push();

			// 右辺を評価
			gen(node->rhs);
			mov(RDI, "0");
			cmp(node->rhs->type, new_primitive_type(TY_INT));
			printf("    setne al\n"); // 0と等しくないかをalに格納

			pop(RDI);
		
			create_add(true, new_primitive_type(TY_INT), new_primitive_type(TY_INT));
			printf("    movzx rax, al\n"); // alをゼロ拡張

			// 最後の比較
			printf(".Lcond%d:\n", lend);
			mov(RDI, "2");
			cmp(new_primitive_type(TY_INT), new_primitive_type(TY_INT));
			printf("    sete al\n"); // 2と等しいかをalに格納
			printf("    movzx rax, al\n"); // alをゼロ拡張
			return ;
		}
		case ND_COND_OR:
		{
			lend = jumpLabelCount++;

			// 左辺を評価
			gen(node->lhs);
			mov(RDI, "0");
			cmp(node->lhs->type, new_primitive_type(TY_INT));
			printf("    setne al\n"); // 0と等しくないかをalに格納
			printf("    movzx rax, al\n"); // alをゼロ拡張
			printf("    jne .Lcond%d\n", lend); // 0以外ならスキップ

			// 右辺を評価
			gen(node->rhs);
			mov(RDI, "0");
			cmp(node->rhs->type, new_primitive_type(TY_INT));
			printf("    setne al\n"); // 0と等しくないかをalに格納
			printf("    movzx rax, al\n"); // alをゼロ拡張

			printf(".Lcond%d:\n", lend);
			return ;
		}
		case ND_BITWISE_OR:
		{
			gen(node->lhs);
			push();
			gen(node->rhs);
			pop(RDI);

			printf("    or %s, %s\n", RAX, RDI);
			return ;
		}
		case ND_SHIFT_RIGHT:
		{
			gen(node->rhs);
			push();
			gen(node->lhs);
			pop(RDI);
			mov(CL, DIL);

			printf("    shr %s, %s\n", RAX, CL);
			return;
		}
		case ND_SHIFT_LEFT:
		{
			gen(node->rhs);
			push();
			gen(node->lhs);
			pop(RDI);
			mov(CL, DIL);

			printf("    shl %s, %s\n", RAX, CL);
			return;
		}
		case ND_EQUAL:
		{
			equality_prologue(node);
			cmp(node->lhs->type, node->rhs->type);
			printf("    sete al\n");
			printf("    movzx rax, al\n");
			return;
		}
		case ND_NEQUAL:
		{
			equality_prologue(node);
			cmp(node->lhs->type, node->rhs->type);
			printf("    setne al\n");
			printf("    movzx rax, al\n");
			return;
		}
		case ND_LESS:
		{
			relational_prologue(node);
			cmp(node->lhs->type, node->rhs->type);
			printf("    setl al\n");
			printf("    movzx rax, al\n");
			return;
		}
		case ND_LESSEQ:
		{
			relational_prologue(node);
			cmp(node->lhs->type, node->rhs->type);
			printf("    setle al\n");
			printf("    movzx rax, al\n");
			return;
		}
		case ND_ADD:
		case ND_SUB:
		{
			add_check_pointer(node->type, &node->lhs, &node->rhs, true);

			gen(node->rhs);
			push();
			gen(node->lhs);
			pop("rdi");

			create_add(node->kind == ND_ADD, node->lhs->type, node->rhs->type);
			return;
		}
		case ND_ADDR:
		{
			switch(node->lhs->kind)
			{
				// 変数ならアドレスを取り出す
				case ND_LVAR:
				case ND_LVAR_GLOBAL:
					lval(node->lhs);
					break ;
				// 構造体, unionからのアクセスなら、アドレスなのでそのまま返す
				case ND_MEMBER_VALUE:
				case ND_MEMBER_PTR_VALUE:
					arrow(node->lhs, true);
					break ;
				// ND_DEREFならアドレスで止める
				case ND_DEREF:
					gen(node->lhs->lhs);//ここ！！　↓
					break ;
				default:
					error("ND_ADDRを使えない kind:%d", node->lhs->kind);
					break ;
			}
			return;
		}
		case ND_DEREF:
		{
			gen(node->lhs);// ここと同じ
			load(node->type);
			return;
		}
		case ND_BITWISE_NOT:
		{
			gen(node->lhs);
			printf("    xor %s, -1\n", RAX);
			return;
		}
		case ND_MEMBER_VALUE:
		case ND_MEMBER_PTR_VALUE:
		{
			arrow(node, false);
			return;
		}
		case ND_MUL:
		{
			mul_prologue(node);
			// TODO 型を考慮する
			printf("    imul rax, rdi\n");	
			return;
		}
		case ND_DIV:
		{
			mul_prologue(node);
			// TODO 型を考慮する
			printf("    cdq\n");
			printf("    idiv edi\n");
			return;
		}
		case ND_MOD:
		{
			mul_prologue(node);
			// TODO 型を考慮する
			printf("    cdq\n"); // d -> q -> o
			printf("    idiv edi\n");
			mov(RAX, RDX);
			return;
		}
		case ND_CAST:
		{
			gen(node->lhs);
			cast(node->lhs->type, node->type);
			return;
		}
		case ND_LVAR:
		{
			lval(node);
			load(node->type);
			return ;
		}
		case ND_LVAR_GLOBAL:
		{
			lval(node);
			load(node->type);
			return;
		}
		case ND_STR_LITERAL:
		{
			printf("    %s %s, [rip + %s]\n", ASM_LEA, RAX,
					get_str_literal_name(node->str_index));
			return;
		}
		case ND_CALL:
		{
			call(node);
			return;
		}
		case ND_NUM:
		{
			movi(RAX, node->val);
			return;
		}
		case ND_PARENTHESES:
		{
			gen(node->lhs);
			return;
		}
		default:
		{
			error("不明なノード kind:%d type:%d", node->kind, node->type->ty);
			return;
		}
	}
}
