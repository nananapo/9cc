#include "9cc.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define ARG_REG_COUNT 6

static void	expr(Node *node);
static void unary(Node *node);

static int	jumpLabelCount = 0;
static char	*arg_regs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
static int	stack_count = 0;

extern t_str_elem	*str_literals;

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

void	push()
{
	stack_count += 8;
	printf("    %s %s # stack= %d -> %d\n", ASM_PUSH, RAX, stack_count - 8, stack_count);
}

void	pushi(int data)
{
	stack_count += 8;
	printf("    %s %d # stack= %d -> %d\n", ASM_PUSH, data, stack_count - 8, stack_count);
}

void	pop(char *reg)
{
	stack_count -= 8;
	printf("    pop %s # stack= %d -> %d\n", reg, stack_count + 8, stack_count);
}

void	mov(char *dst, char *from)
{
	printf("    %s %s, %s\n", ASM_MOV, dst, from);
}

void	movi(char *dst, int i)
{
	printf("    %s %s, %d\n", ASM_MOV, dst, i);
}

void	cmps(char *dst, char *from)
{
	printf("    cmp %s, %s\n", dst, from);
}

// TODO 構造体の比較
// rax, rdi
void	cmp(Type *dst, Type *from)
{
	if (type_equal(dst, from))
	{
		if (dst->ty == PTR || dst->ty == ARRAY)
			cmps(RAX, RDI);
		else if (dst->ty == CHAR)
			cmps(AL, DIL);
		else if (dst->ty == INT)
			cmps(EAX, EDI);
		return ;
	}
	cmps(RAX, RDI);
}

void	load_global(Node *node)
{
	char	*prefix;

	printf("    mov rax, [rip + _%s@GOTPCREL]\n",
		strndup(node->var_name, node->var_name_len));
}

char	*get_str_literal_name(int index)
{
	char	*tmp;

	tmp = calloc(100, sizeof(char));
	sprintf(tmp, "L_STR_%d", index);
	return (tmp);
}

void	comment(char *c)
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

	op = "+";
	if (minus_step)
		op = "-";

	dest = R10;
	
	for (int i = 0; i < size; i += 8)
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

static void prologue()
{
	// prologue
	mov(RAX, RBP);
	push();
	mov(RBP, RSP);
}

static void	epilogue()
{
	// epilogue
	mov(RSP, RBP);
	pop(RBP);
	printf("    ret\n");
}

// raxをraxに読み込む
static void	load(Type *type)
{
	
	if (type->ty == PTR)
	{
		mov(RAX, "[rax]");
		return ;
	}
	else if (type->ty == CHAR)
	{
		printf("    mov al, %s [%s]\n", BYTE_PTR, RAX);
		printf("    movzx %s, %s\n", RAX, AL);
		return ;
	}
	else if (type->ty == INT)
	{
		printf("    mov %s, %s [%s]\n", EAX, DWORD_PTR, RAX);
		//printf("    movzx %s, %s\n", RAX, EAX);
		return ;
	}
	else if (type->ty == ARRAY)
	{
		return ;
	}
	else if (type->ty == STRUCT)
	{
		// TODO とりあえず8byteまで
		// mov(RAX, "[rax]");
		return ;
	}
}

// 変数のアドレスをraxに移動する
static void	lval(Node *node)
{
	if (node->kind != ND_LVAR
	&& node->kind != ND_LVAR_GLOBAL)
		error("変数ではありません Kind:%d Type:%d", node->kind, node->type->ty);
	
	mov(RAX, RBP);
	if (node->offset > 0)
		printf("    sub %s, %d\n", RAX, node->offset);
	else if (node->offset < 0)
		printf("    add %s, %d\n", RAX, - node->offset);
}

static void	call(Node *node)
{
	int		size;
	int 	rbp_offset;
	int		arg_count = 0;
	int		push_count = 0;
	bool	is_aligned;

	for (Node *tmp = node->args; tmp; tmp = tmp->next)
	{
		arg_count++;

		size = type_size(tmp->type);
		printf("# PUSH ARG %s (%d)\n",
			strndup(tmp->locals->name, tmp->locals->len),
			size);

		expr(tmp);

		// TODO long doubleで動かなくなります....
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
			for (int i = 0; i < size; i += 8)
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
	for (Node *tmp = node->args; tmp; tmp = tmp->next)
	{
		if (tmp->locals->arg_regindex != -1)
			continue ;
		rbp_offset = min(rbp_offset,
						tmp->locals->offset - align_to(type_size(tmp->locals->type), 8) + 8);
	}

	// マイナスなのでプラスにする
	rbp_offset = - rbp_offset;

	is_aligned = (stack_count + rbp_offset + 8) % 16 == 0;
	if (!is_aligned)
		rbp_offset += 8;

	printf("# RBP_OFFSET %d (is_aligned : %d)\n", rbp_offset, is_aligned);

	int	pop_count = 0;
	// 後ろから格納していく
	for (int i = arg_count; i > 0; i--)
	{
		// i番目のtmpを求める
		Node *tmp = node->args;
		for (int j = 1; j < i; j++)
			tmp = tmp->next;

		printf("# POP %s\n", strndup(tmp->locals->name, tmp->locals->len));
	
		size = type_size(tmp->type);

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
			for (int i = size - 8; i >= 0; i -= 8)
			{
				printf("    %s %s, [%s + %d]\n", ASM_MOV,
					arg_regs[tmp->locals->arg_regindex - i / 8],
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
		if (tmp->type->ty != STRUCT)
			store_value(size);
		else
			store_ptr(size, false);
	}

	// rspを移動する
	if (rbp_offset != 0)
	{
		if (!is_aligned)
			comment("aligned + 8");
		printf("    sub rsp, %d # rbp_offset\n", rbp_offset);
	}

	// call
	printf("# CALL RBP_OFFSET: %d\n", rbp_offset);
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

	return;
}

// raxに入っている型fromをtoに変換する
static void	cast(Type *from, Type *to)
{
	int	size1;
	int	size2;

	// 型が同じなら何もしない
	if (type_equal(from, to))
		return ;

	// ポインタからポインタのキャストは何もしない
	if (is_pointer_type(from)
	&& is_pointer_type(to))
		return ;
	
	size1 = type_size(from);
	size2 = type_size(to);

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
				printf("    movsxd %s, %s\n", RAX, AL);
			else if (size1 == 2)
				printf("    movsxd %s, %s\n", RAX, AX);
			else if (size1 == 4)
				printf("    movsxd %s, %s\n", RAX, EAX);
			else
				error("8byte -> 8byteのキャストは無い");
		}
		return ;
	}

	error("%sから%sへのキャストが定義されていません", get_type_name(from), get_type_name(to));
}

static void	primary(Node *node)
{
	char *ch;

	switch (node->kind)
	{
		case ND_CAST:
			unary(node->lhs);
			cast(node->lhs->type, node->type);
			return;
		case ND_LVAR:
			lval(node);
			load(node->type);
			return;
		case ND_LVAR_GLOBAL:
			load_global(node);
			load(node->type);
			return;
		case ND_STR_LITERAL:
			printf("    %s %s, [rip + %s]\n", ASM_LEA, RAX,
					get_str_literal_name(node->str_index));
			return;
		case ND_CALL:
			call(node);
			return;
		case ND_NUM:
			movi(RAX, node->val);
			return;
		case ND_PROTOTYPE:
		case ND_DEFVAR:
			return;
		case ND_STRUCT_DEF:
			error("おかしい");
			return;
		default:
			error("不明なノード %d", node->kind);
			break;
	}
}

static void	arrow(Node *node, bool as_addr)
{
	int offset;

	switch (node->kind)
	{
		case ND_STRUCT_VALUE:
		case ND_STRUCT_PTR_VALUE:
			break;
		default:
			return primary(node);
	}

	//printf("#ARROW %d->%d\n", node->kind, node->lhs->kind);

	// arrowかその他の可能性がある
	if (node->lhs->kind == ND_STRUCT_VALUE
			|| node->lhs->kind == ND_STRUCT_PTR_VALUE)
		arrow(node->lhs, node->kind == ND_STRUCT_VALUE);
	else
		expr(node->lhs);

	// offset分ずらす => 最適化で消えるので消さなくてもいいかも
	offset = node->struct_elem->offset;
	if (offset != 0)
		printf("    add rax, %d\n", offset);

	// 値として欲しいなら値にする
	if (!as_addr)
		load(node->struct_elem->type);
}

static void unary(Node *node)
{
	switch (node->kind)
	{
		case ND_ADDR:
		case ND_DEREF:
			break ;
		default:
			arrow(node, false);
			return ;
	}

	switch(node->kind)
	{
		case ND_ADDR:
			switch(node->lhs->kind)
			{
				// 変数ならアドレスを取り出す
				case ND_LVAR:
				case ND_LVAR_GLOBAL:
					lval(node->lhs);
					break ;
				// 構造体からのアクセスなら、アドレスなのでそのまま返す
				case ND_STRUCT_VALUE:
				case ND_STRUCT_PTR_VALUE:
					break ;
				// ND_DEREFならアドレスで止める
				case ND_DEREF:
					expr(node->lhs->lhs);//ここ！！　↓
					break ;
				default:
					error("ND_ADDRを使えない kind:%d", node->lhs->kind);
					break ;
			}
			break ;
		case ND_DEREF:
			expr(node->lhs);// ここと同じ！！！！！変えるときは注意！！！！！！
			load(node->type);
			break ;
		default:
			break ;
	}
}

static void	mul(Node *node)
{
	switch (node->kind)
	{
		case ND_MUL:
		case ND_DIV:
			break;
		default:
			unary(node);
			return;
	}

	expr(node->rhs);
	push();
	expr(node->lhs);
	pop("rdi");

	switch (node->kind)
	{
		case ND_MUL:
			printf("    imul rax, rdi\n");	
/*
			if (node->type->ty == INT)
			{
				printf("    imul eax, edi\n");	
			}
*/
			break;
		case ND_DIV:
			printf("    cqo\n");
			printf("    idiv rdi\n");
			break;
		default:
			break;
	}
}

static void	add(Node *node)
{
	switch (node->kind)
	{
		case ND_ADD:
		case ND_SUB:
			break;
		default:
			mul(node);
			return;
	}
	
	// 結果がポインタ型なら
	if (node->type->ty == PTR
	|| node->type->ty == ARRAY)
	{
		// 左辺をポインタ型にする
		if (node->rhs->type->ty == PTR || node->rhs->type->ty == ARRAY)
		{
			Node *tmp = node->lhs;
			node->lhs = node->rhs;
			node->rhs = tmp;
		}

		// 右辺が整数型なら掛け算に置き換える
		if (is_integer_type(node->rhs->type))
		{
			Node *size_node = new_node(ND_NUM, NULL, NULL);
			size_node->val = type_size(node->lhs->type->ptr_to);
			size_node->type = new_primitive_type(INT);
			// 掛け算
			node->rhs = new_node(ND_MUL, node->rhs, size_node);
			node->rhs->type = node->lhs->type; // TODO
			// INT
			size_node->type = new_primitive_type(INT);
		}
	}

	expr(node->rhs);
	push();
	expr(node->lhs);
	pop("rdi");

	switch (node->kind)
	{
		case ND_ADD:
			printf("    add rax, rdi\n");
			break;
		case ND_SUB:
			printf("    sub rax, rdi\n");
			break;
		default:
			break;
	}
}

static void relational(Node *node)
{
	switch (node->kind)
	{
		case ND_LESS:
		case ND_LESSEQ:
			break;
		default:
			add(node);
			return;
	}

	expr(node->rhs);
	push();
	expr(node->lhs);
	pop("rdi");

	switch (node->kind)
	{
		case ND_LESS:
			cmp(node->lhs->type, node->rhs->type);
			printf("    setl al\n");
			printf("    movzx rax, al\n");
			break;
		case ND_LESSEQ:
			cmp(node->lhs->type, node->rhs->type);
			printf("    setle al\n");
			printf("    movzx rax, al\n");
			break;
		default:
			break;
	}
}

static void	equality(Node *node)
{
	switch (node->kind)
	{
		case ND_EQUAL:
		case ND_NEQUAL:
			break;
		default:
			relational(node);
			return;
	}

	expr(node->rhs);
	push();
	expr(node->lhs);
	pop("rdi");

	switch (node->kind)
	{
		case ND_EQUAL:
			cmp(node->lhs->type, node->rhs->type);
			printf("    sete al\n");
			printf("    movzx rax, al\n");
			break;
		case ND_NEQUAL:
			cmp(node->lhs->type, node->rhs->type);
			printf("    setne al\n");
			printf("    movzx rax, al\n");
			break;
		default:
			break;
	}
}

static void	assign(Node *node)
{
	int	size;

	if (node->kind != ND_ASSIGN)
	{
		equality(node);
		return;
	}

	//printf("#ASSIGN %d\n", node->lhs->kind);
	
	if (node->lhs->kind == ND_LVAR)
		lval(node->lhs);
	else if (node->lhs->kind == ND_LVAR_GLOBAL)
		load_global(node->lhs);
	else if (node->lhs->kind == ND_DEREF)
		expr(node->lhs->lhs);// ここもDEREFと同じようにやってる！！！！！
	else if (node->lhs->kind == ND_STRUCT_VALUE)
		arrow(node->lhs, true);
	else if (node->lhs->kind == ND_STRUCT_PTR_VALUE)
		arrow(node->lhs, true);
	else
		error("代入の左辺値が識別子かアドレスではありません");

	push();
	expr(node->rhs);
	pop("r10");

	// storeする
	if (node->type->ty == ARRAY)
		// TODO これOKなの？
		// ARRAYに対する代入がうまくいかない気がする
		store_value(8);
	else if(node->type->ty == STRUCT)
		store_ptr(type_size(node->type), false);
	else
		store_value(type_size(node->type));
}

static void	expr(Node *node)
{
	assign(node);
}

static void stmt(Node *node)
{
	int	lend;
	int	lbegin;

	switch (node->kind)
	{
		case ND_RETURN:
		case ND_IF:
		case ND_WHILE:
		case ND_FOR:
		case ND_BLOCK:
			break;
		default:
			expr(node);
			return;
	}

	switch (node->kind)
	{
		case ND_RETURN:
			expr(node->lhs);
			epilogue();
			stack_count += 8; // rbpをpopしたけれど、epilogueでもpopするので+8
			return;
		case ND_IF:
			// if
			expr(node->lhs);
			mov(RDI, "0");
			cmp(node->lhs->type, new_primitive_type(INT));

			lend = jumpLabelCount++;

			if (node->els == NULL)
			{
				printf("    je .Lend%d\n", lend);
				stmt(node->rhs);
			}
			else
			{
				int lelse = jumpLabelCount++;
				printf("    je .Lelse%d\n", lelse);
				
				// if stmt
				stmt(node->rhs);
				printf("    jmp .Lend%d\n", lend);

				// else
				printf(".Lelse%d:\n", lelse);
				stmt(node->els);
			}
			printf(".Lend%d:\n", lend);
			return;
		case ND_WHILE:
			lbegin = jumpLabelCount++;
			lend = jumpLabelCount++;
			
			printf(".Lbegin%d:\n", lbegin);
			
			// if
			expr(node->lhs);
			mov(RDI, "0");
			cmp(node->lhs->type, new_primitive_type(INT));
			printf("    je .Lend%d\n", lend);
			
			// while block
			stmt(node->rhs);
			
			// next
			printf("    jmp .Lbegin%d\n", lbegin);
			
			// end
			printf(".Lend%d:\n", lend);
			return;
		case ND_FOR:
			lbegin = jumpLabelCount++;
			lend = jumpLabelCount++;
			
			// init
			if (node->for_init != NULL)
				expr(node->for_init);

			printf(".Lbegin%d:\n", lbegin);
			
			// if
			if(node->for_if != NULL)
			{
				expr(node->for_if);
				mov(RDI, "0");
				cmp(node->for_if->type, new_primitive_type(INT));
				printf("    je .Lend%d\n", lend);
			}

			// for-block
			stmt(node->lhs);

			// next
			if(node->for_next != NULL)
				expr(node->for_next);

			printf("    jmp .Lbegin%d\n", lbegin);
			
			//end
			printf(".Lend%d:\n", lend);
			return;
		case ND_BLOCK:
			while(node != NULL)
			{
				if (node->lhs == NULL)
					break;
				stmt(node->lhs);
				node = node->rhs;
			}
			return;
		default:
			break;
	}
}

static void	funcdef(Node *node)
{
	char	*funcname;

	funcname = strndup(node->fname, node->flen);
	stack_count = 0;

	printf(".globl _%s\n", funcname);
	printf("_%s:\n", funcname);	
	prologue();

	if (node->locals != NULL)
	{
		int maxoff = 0;
		for (LVar *tmp = node->locals; tmp; tmp = tmp->next)
		{
			printf("# VAR %s %d\n", strndup(tmp->name, tmp->len), tmp->offset);
			maxoff = max(maxoff, tmp->offset);
		}
		node->stack_size = align_to(maxoff, 8);
	}
	else
		node->stack_size = 0;

	stack_count += node->stack_size; // stack_sizeを初期化

	printf("# STACKSIZE : %d\n", node->stack_size);

	if (node->stack_size != 0)
	{
		printf("    sub rsp, %d\n", node->stack_size);// stack_size

		for (LVar *tmp = node->locals; tmp; tmp = tmp->next)
		{
			if (!tmp->is_arg)
				continue ;
			printf("# ARG %s\n", strndup(tmp->name, tmp->len));
			if (tmp->arg_regindex != -1)
			{
				int index = tmp->arg_regindex;
				int size = align_to(type_size(tmp->type), 8);

				mov(R10, RBP);
				printf("    sub %s, %d\n", R10, tmp->offset);

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

	stmt(node->lhs);
	epilogue();

	stack_count -= node->stack_size;
	printf("#count %d\n", stack_count);
	
	if (stack_count != 0)
		error("stack_countが0ではありません");
}

static void globaldef(Node *node)
{
	char	*name;

	name = strndup(node->var_name, node->var_name_len);
	printf(".globl %s\n", name);
	printf("    .zerofill __DATA,__common,_%s,%d,2\n",
		name,
		type_size(node->type));
}

static void	filescope(Node *node)
{
	if (node->kind == ND_FUNCDEF)
		funcdef(node);
	else if (node->kind == ND_DEFVAR_GLOBAL)
		globaldef(node);
	else
		stmt(node);
}

void	gen(Node *node)
{
	if (node->kind == ND_PROTOTYPE
	|| node->kind == ND_DEFVAR
	|| node->kind == ND_STRUCT_DEF)
		return;
	filescope(node);
}
