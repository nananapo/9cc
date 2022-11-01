#include "9cc.h"
#include "il.h"
#include "charutil.h"
#include "mymath.h"
#include "veri.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define BYTE2BIT 8

// main
extern t_deffunc		*g_func_defs[1000];
extern t_defvar			*g_global_vars[1000];
extern t_str_elem		*g_str_literals[1000];
extern t_il				*g_il;

static int				g_proc_count;
static t_veriproc		*g_funcend_state;


static void	translate_process(t_node *node, t_veriproc **head, t_veriproc **tail);

///////////////////////////////////////////////////////////////
//--------------------ABI--------------------------------------
int	get_array_align_size_verilog(t_type *type)
{
	return (get_type_size(type));
}

int	get_type_size_verilog(t_type *type)
{
	switch (type->ty)
	{
		case TY_INT:
			return (4);
		case TY_CHAR:
			return (1);
		case TY_BOOL:
			return (1);
		case TY_FLOAT:
			return (4);
		case TY_DOUBLE:
			return (4);
		case TY_PTR:
			return (8);
		case TY_ARRAY:
			return (get_type_size(type->ptr_to) * type->array_size);
		case TY_VOID:
			return (1);
		case TY_ENUM:
			return  (4);
		case TY_STRUCT:
		case TY_UNION:
			error("Not impl : size of struct / union%d", type->ty);
		default:
			error("size of unknown type %d", type->ty);
	}
	return (-1);
}

bool	is_unsigned_abi_verilog(t_typekind kind)
{
	switch (kind)
	{
		case TY_PTR:
		case TY_ARRAY:
			return (true);
		default:
			return (false);
	}
	return (false);
}

//--------------------End of ABI-------------------------------
///////////////////////////////////////////////////////////////



// veriprocを追加
static t_veriproc	*create_proc(t_verikind kind)
{
	int			id;
	t_veriproc	*proc;

	id = g_proc_count++;
	proc = calloc(1, sizeof(t_veriproc));
	proc->kind = kind;
	proc->state_id = id;
	proc->next = NULL;
	return (proc);
}

static void	translate_process(t_node *node, t_veriproc **head, t_veriproc **tail)
{
	t_veriproc	*proc;
	t_veriproc	*tmp_head;
	t_veriproc	*tmp_tail;

	proc = NULL;
	*head = NULL;
	*tail = NULL;
	tmp_head = NULL;
	tmp_tail = NULL;

	switch (node->kind)
	{
		case ND_NONE:
			return ;
		case ND_ASSIGN:
		{
			// ここの書き方、非常に良い :)
			translate_process(node->lhs, head, &tmp_head);
			translate_process(node->rhs, &tmp_tail, tail);
			tmp_head->next = tmp_tail;

			proc = create_proc(VERI_ASSIGN);
			proc->type = node->type;

			(*tail)->next = proc;
			*tail = proc;
			return ;
		}
		case ND_NUM:
		{
			proc = create_proc(VERI_NUM);
			proc->type = node->type;
			proc->num = node->val;
			*head = proc;
			*tail = proc;
			return ;
		}
		case ND_VAR_LOCAL_ADDR:
		{
			proc = create_proc(VERI_ADDRESS);
			proc->num = node->lvar->offset;
			*head = proc;
			*tail = proc;
			return ;
		}
		case ND_VAR_LOCAL:
		{
			proc = create_proc(VERI_ADDRESS);
			proc->num = node->lvar->offset;
			*head = proc;

			proc = create_proc(VERI_LOAD);
			*tail = proc;
			(*head)->next = proc;
			return ;
		}
		case ND_BLOCK:
		{
			for (; node != NULL && node->lhs != NULL; node = node->rhs)
			{
				translate_process(node->lhs, &tmp_head, &tmp_tail);
				if (tmp_head == NULL)
					continue ;
				if (*head == NULL)
				{
					*head = tmp_head;
					*tail = tmp_tail;
				}
				else
				{
					(*tail)->next = tmp_head;
					*tail = tmp_tail;
				}
			}
			return ;
		}
		case ND_RETURN:
		{
			// 返り値なしreturn
			if (node->lhs == NULL)
			{
				proc = create_proc(VERI_JUMP);
				proc->next = g_funcend_state;
				*head = proc;
				*tail = proc;
				return ;
			}

			translate_process(node->lhs, head, tail);
			proc = create_proc(VERI_RETURN);
			(*tail)->next = proc;
			*tail = proc;
			return ;
		}
		case ND_ADD:
		{
			translate_process(node->lhs, head, &tmp_head);
			translate_process(node->rhs, &tmp_tail, tail);
			tmp_head->next = tmp_tail;

			proc = create_proc(VERI_ADD);
			proc->type = node->type;

			(*tail)->next = proc;
			*tail = proc;
			return ;
		}
		case ND_SUB:
		{
			translate_process(node->lhs, head, &tmp_head);
			translate_process(node->rhs, &tmp_tail, tail);
			tmp_head->next = tmp_tail;

			proc = create_proc(VERI_SUB);
			proc->type = node->type;

			(*tail)->next = proc;
			*tail = proc;
			return ;
		}
		case ND_CAST:
		{
			translate_process(node->lhs, head, tail);
			
			proc = create_proc(VERI_CAST);
			proc->type = node->type;
			proc->cast_from = node->lhs->type;

			(*tail)->next = proc;
			*tail = proc;
			return ;
		}
		default:
		{
			printf("noimpl %d\n", node->kind);
			exit(1);
			return ;
		}
	}
}

static void	gen_process(t_veriproc *proc)
{
	printf(" else if (state == 32'd%d) begin\n", proc->state_id);

	printf("  // id:%d\n", proc->kind);
	// next state
	switch (proc->kind)
	{
		case VERI_RETURN:
			break ;
		default:
			if (proc->next != NULL)
				printf("  state <= 32'd%d;\n", proc->next->state_id);
			break ;
	}

	// main loginc
	switch (proc->kind)
	{
		case VERI_NUM:
		case VERI_ADDRESS:
			printf("  sp = sp + 32'd1;\n");
			printf("  stack[sp] = 32'd%d;\n", proc->num);
			break ;
		case VERI_ASSIGN:
			printf("  r0 = stack[sp];\n");
			printf("  r1 = stack[sp-32'd1];\n");
			printf("  stack[r1] = r0;\n");
			printf("  sp = sp - 32'd1;\n");
			printf("  stack[sp] = r0;\n");
			break ;
		case VERI_FUNC_END:
			printf("  // funcend\n");
			break ;
		case VERI_JUMP:
			break ;
		case VERI_LOAD:
			printf("  r0 = stack[sp];\n");
			printf("  r1 = stack[r0];\n");
			printf("  stack[sp] = r1;\n");
			break ;
		case VERI_CAST:
			// printf("%s -> %s\n", get_type_name(proc->cast_from), get_type_name(proc->type));
			// TODO
			break ;
		case VERI_RETURN:
			printf("  ret = stack[sp];\n");
			printf("  sp = sp - 32'd1;\n");
			printf("  state <= 32'd%d;\n", g_funcend_state->state_id); // TODO nextで繋いでおきたい
			break ;
		case VERI_ADD:
			printf("  r0 = stack[sp];\n");
			printf("  r1 = stack[sp-32'd1];\n");
			printf("  sp = sp - 32'd1;\n");
			printf("  stack[sp] = r0 + r1;\n");
			break ;
		case VERI_SUB:
			printf("  r0 = stack[sp];\n");
			printf("  r1 = stack[sp-32'd1];\n");
			printf("  sp = sp - 32'd1;\n");
			printf("  stack[sp] = r1 - r0;\n");
			break ;
		default:
			printf("noimpl %d\n", proc->kind);
			exit(1);
	}
	printf(" end");
}

#define STACK_SIZE 1024

static void	gen_func(t_deffunc *func)
{
	t_lvar		*lvar;
	int			val_offset;
	t_veriproc	*funcend;
	t_veriproc	*funcproc_head;
	t_veriproc	*funcproc_tail;

	// module
	printf("module %s(\n", my_strndup(func->name, func->name_len));
	printf("    input wire clock,\n");
	printf("    input wire n_reset\n");
	printf(");\n\n"); // 引数は考えない

	// 変数
	printf("reg [31:0] state;\n");
	printf("reg [31:0] stack[0:%d];\n", STACK_SIZE - 1);
	printf("reg [31:0] sp;\n");
	printf("reg [31:0] r0;\n");
	printf("reg [31:0] r1;\n");
	printf("reg [31:0] r2;\n");
	printf("reg [31:0] ret;\n");
	val_offset = 0;
	for (lvar = func->locals; lvar != NULL; lvar = lvar->next)
	{
		int	size = get_type_size(lvar->type);
		val_offset += size;
		lvar->offset = val_offset;
		// regに保存するならコレ
		//char	*name = my_strndup(lvar->name, lvar->name_len);
		//printf("reg [%d:0] val_%s;\n", size * 8 - 1, name);
	}
	printf("\n");

	// 終了状態を作成
	funcend = create_proc(VERI_FUNC_END);
	g_funcend_state = funcend;

	// 状態を生成
	translate_process(func->stmt, &funcproc_head, &funcproc_tail);

	printf("always @ (posedge clock, negedge n_reset) begin\n");
	printf(" if (n_reset == 1'b0) begin\n"); // reset
	if (funcproc_head != NULL)
		printf("  state <= 32'd%d;\n", funcproc_head->state_id);
	else
		printf("  state <= 32'd%d\n", funcend->state_id);
	printf("  sp <= 32'd%d;\n", val_offset); // スタックポインタ
	printf(" end");

	for (; funcproc_head != NULL;)
	{
		gen_process(funcproc_head);
		funcproc_head = funcproc_head->next; // TODO ifとかは分岐する
	}

	printf("\nend\n");
	

	printf("endmodule\n");
}

void	codegen_verilog(void)
{
	int	i;

	for (i = 0; g_func_defs[i] != NULL; i++)
		gen_func(g_func_defs[i]);
}
