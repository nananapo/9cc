#include "il.h"
#include "optimize.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

extern t_funcil_pair	*g_func_ils;

static t_basicblock		*g_basicblock;
static t_il				*g_il;
static t_il				*g_il_last;
static int				BASICBLOCK_ID;
static t_pair_ilblock	*g_all_blocks;

static void	append_ilblock_pair(t_pair_ilblock **pairs, t_basicblock *block);


static t_basicblock	*create_basicblock(t_il *start, t_il *end, t_basicblock *next, t_basicblock *next_if)
{
	t_basicblock	*block;

	block = calloc(1, sizeof(t_basicblock));
	block->start = start;
	block->end = end;
	block->next = next;
	block->next = next_if;
	block->uniqueid = BASICBLOCK_ID++;

	// save
	append_ilblock_pair(&g_all_blocks, block);
	return (block);
}

static void	append_ilblock_pair(t_pair_ilblock **pairs, t_basicblock *block)
{
	t_pair_ilblock	*p;

	p = calloc(1, sizeof(t_pair_ilblock));
	p->il = block->start;
	p->block = block;
	p->next = *pairs;
	*pairs = p;
	return ;
}

static t_basicblock *search_ilblock_pair(t_pair_ilblock *pairs, t_il *il)
{
	for (; pairs != NULL; pairs = pairs->next)
	{
		if (pairs->il == il)
			return (pairs->block);
	}
	return (NULL);
}

static t_basicblock *search_ilblock_with_label(t_pair_ilblock *pairs, char *name)
{
	for (; pairs != NULL; pairs = pairs->next)
	{
		if (pairs->il->kind == IL_LABEL
		&& strcmp(pairs->il->label_str, name) == 0)
			return (pairs->block);
	}
	return (NULL);
}

static void	connect_basicblock_recursively(t_pair_ilblock *pairs, t_basicblock *block)
{
	t_basicblock *target_block;

	if (block == NULL || block->is_constructed)
		return ;

	block->is_constructed = true;
	if (block->end->kind != IL_JUMP)
	{
		target_block = search_ilblock_pair(pairs, block->end->next);
		block->next = target_block;

		if (target_block != NULL)
			debug("# block(%d)->next = %d", block->uniqueid, target_block->uniqueid);

		connect_basicblock_recursively(pairs, target_block);
	}

	if (block->end->kind == IL_JUMP
	|| block->end->kind == IL_JUMP_TRUE
	|| block->end->kind == IL_JUMP_FALSE)
	{
		target_block = search_ilblock_with_label(pairs, block->end->label_str);

		if (target_block != NULL)
			debug("# block(%d)->next_if = %d", block->uniqueid, target_block->uniqueid);
		else
			debug("# failed to find Label:\"%s\" , block(%d,kind:%d)",
						block->end->label_str, block->uniqueid, block->end->kind);

		block->next_if = target_block;
		connect_basicblock_recursively(pairs, target_block);
	}
}

static t_basicblock	*construct_basicblock(void)
{
	t_basicblock	*block;
	t_basicblock	*block_now;
	t_il			*code;
	t_pair_ilblock	*il_leader_pairs;
	t_basicblock	*headblock;

	block			= NULL;
	block_now		= NULL;
	code			= NULL;
	il_leader_pairs	= NULL;
	headblock		= NULL;

	debug("# start construct");

	// 筆頭と中間表現のペアを作る
	// ラベルをとりあえず筆頭にする
	for (code = g_il; code != NULL; code = code->next)
	{
		if (code->kind == IL_JUMP
		|| code->kind == IL_JUMP_TRUE
		|| code->kind == IL_JUMP_FALSE)
		{
			if (block_now == NULL)
			{
				block = create_basicblock(code, code, NULL, NULL);
				block_now = block;
				append_ilblock_pair(&il_leader_pairs, block);

				debug("# Create Block(%d,kind:%d) start:%d", block->uniqueid, code->kind, code->ilid_unique);
			}
			else
				block_now->end = code;
			block_now = NULL;
		}
		else if (code->kind == IL_LABEL || block_now == NULL)
		{
			block = create_basicblock(code, NULL, NULL, NULL);
			block_now = block;
			append_ilblock_pair(&il_leader_pairs, block);

			debug("# Create Block(%d, kind:%d) start:%d", block->uniqueid, code->kind, code->ilid_unique);
		}
		if (block_now != NULL)
			block_now->end = code;
	}

	debug("# start connect");
	// 基本ブロックをつなぐ
	headblock = search_ilblock_pair(il_leader_pairs, g_il);
	connect_basicblock_recursively(il_leader_pairs, headblock);

	return headblock;
}

static void	mark_next_blocks(t_basicblock *parent, t_basicblock *target)
{
	if (target == NULL
	|| target->il_generated
	|| target->mark_block != NULL)
		return ;

	for (; target != NULL; target = target->next)
	{
		target->mark_block = parent;
	}

	for (; target != NULL; target = target->next)
	{
		mark_next_blocks(target, target->next_if);
	}
}

static void	generate_basicblock_il(t_basicblock *block)
{
	t_basicblock	*tmp;

	if (block == NULL || block->il_generated)
		return ;

	debug("# call(%d)", block->uniqueid);

	// generate block
	for (tmp = block; tmp != NULL; tmp = tmp->next)
	{
		debug("# generate(%d)", tmp->uniqueid);
		if (g_il == NULL)
		{
			g_il = tmp->start;
			g_il_last = tmp->end;
		}
		else
		{
			g_il_last->next = tmp->start;
			g_il_last = tmp->end;
		}
		tmp->il_generated = true;
		g_il_last->next = NULL;
	}

	for (tmp = block; tmp != NULL; tmp = tmp->next)
	{
		if (tmp->next_if == NULL)
			continue ;
		mark_next_blocks(tmp, tmp->next_if);
	}

	for (tmp = block; tmp != NULL; tmp = tmp->next)
	{
		if (tmp->next_if == NULL
		|| tmp->next_if->mark_block != tmp)
			continue ;
		generate_basicblock_il(tmp->next_if);
	}	
}

static bool	delete_cast_nonsense(void)
{
	t_il	*code;
	bool	updated;

	updated = false;
	for (code = g_il; code != NULL; code = code->next)
	{
		if (code->kind != IL_CAST)
			continue ;
		if (type_equal(code->cast_from, code->cast_to))
		{
			updated = true;
			if (code->before != NULL)
				code->before->next = code->next;
			if (code->next != NULL)
				code->next->before = code->before;
			if (code == g_il)
				g_il = code->next;
		}
	}
	return (updated);
}

static bool	delete_push_pop(void)
{
	t_il	*code;
	bool	updated;

	updated = false;
	for (code = g_il; code != NULL; code = code->next)
	{
		if (code->before == NULL
		|| code->kind != IL_POP)
			continue ;

		switch (code->before->kind)
		{
			case IL_PUSH_AGAIN:
			case IL_PUSH_NUM:
			case IL_PUSH_FLOAT:
			case IL_VAR_LOCAL:
			case IL_VAR_LOCAL_ADDR:
			case IL_VAR_GLOBAL:
			case IL_VAR_GLOBAL_ADDR:
			case IL_MEMBER:
			case IL_MEMBER_ADDR:
			case IL_MEMBER_PTR:
			case IL_MEMBER_PTR_ADDR:
			case IL_STR_LIT:
				// loadとか、そもそも結果を使わなくなるものはデータフロー解析で消す
				// というか、データフロー解析するならこの処理いらない
				break ;
			default:
				continue ;
		}

		updated = true;
		if (code->before->before != NULL)
			code->before->before->next = code->next;
		if (code->next != NULL)
			code->next->before = code->before->before;
		if (code->before == g_il)
			g_il = code->next;
	}
	return (updated);
}

static bool	delete_label_array(void)
{
	t_il		*code;
	t_il		*head;
	t_label_pair*pairs;
	t_label_pair*tmp;
	bool		updated;

	head = NULL;
	pairs = NULL;
	tmp = NULL;
	updated = false;

	// ラベルをまとめる
	for (code = g_il; code != NULL; code = code->next)
	{
		if (code->before == NULL
		|| code->kind != IL_LABEL)
			continue ;

		head = code;
		for (code = code->next;
			code != NULL && code->kind == IL_LABEL;
			code = code->next)
		{
			tmp = calloc(1, sizeof(t_label_pair));
			tmp->from = code->label_str;
			tmp->to = head->label_str;
			tmp->next = pairs;
			pairs = tmp;
			updated = true;
		}
		head->next = code;
	}

	// ラベルを置き換える
	for (code = g_il; code != NULL; code = code->next)
	{
		if (code->label_str == NULL)
			continue ;
		for (tmp = pairs; tmp != NULL; tmp = tmp->next)
		{
			if (strcmp(code->label_str, tmp->from) == 0)
			{
				code->label_str = tmp->to;
				break ;
			}
		}
	}

	return (updated);
}

static void	debug_il(void)
{
	t_il	*tmpil;

	debug("# printil start");
	for (tmpil = g_il; tmpil != NULL; tmpil = tmpil->next)
	{
		print_il(tmpil);
	}
	debug("# printil end");
}

static void debug_basicblock(FILE *fp, t_basicblock *block)
{
	t_il	*code;

	fprintf(fp, "Block #%d\n", block->uniqueid);

	code = block->start;
	for (; code != block->end; code = code->next)
		fprintf(fp, "#%d %s\\l", code->ilid_unique, get_il_name(code->kind));
	fprintf(fp, "#%d %s\\l", code->ilid_unique, get_il_name(code->kind));
}

static void debug_basicblock_dot(char *funcname)
{
	FILE			*fp;
	t_pair_ilblock	*pair;
	char			filename[1000];

	filename[0] = '\0';
	strcat(filename, "dot/");
	strcat(filename, funcname);
	strcat(filename, ".dot");

	fp = fopen(filename, "w");
	if (fp == NULL)
		return ;

	fprintf(fp,"digraph graph_name {\n");
 	fprintf(fp," graph [\n");
 	fprintf(fp,"   charset = \"UTF-8\";\n");
 	fprintf(fp,"   fontsize = 18,\n");
 	fprintf(fp,"   style = \"filled\",\n");
 	fprintf(fp,"   margin = 0.2,\n");
 	fprintf(fp,"   layout = dot\n");
 	fprintf(fp," ];\n");
 	fprintf(fp," node [\n");
 	fprintf(fp,"   shape = rect,\n");
 	fprintf(fp,"   fontsize = 14,\n");
 	fprintf(fp," ];\n");
 	fprintf(fp," edge [\n");
 	fprintf(fp,"   color = black\n");
 	fprintf(fp," ];\n");

	for (pair = g_all_blocks; pair != NULL; pair = pair->next)
	{
		if (!pair->block->il_generated)
			continue ;

		if (pair->block->next != NULL)
		{
			fprintf(fp, "\"");
				debug_basicblock(fp, pair->block);
			fprintf(fp, "\" -> \"");
				debug_basicblock(fp, pair->block->next);
			fprintf(fp, "\"[label = \"next\"];");
		}

		if (pair->block->next_if != NULL)
		{
			fprintf(fp, "\"");
				debug_basicblock(fp, pair->block);
			fprintf(fp, "\" -> \n\"");
				debug_basicblock(fp, pair->block->next_if);
			fprintf(fp, "\"[label = \"");
			if (pair->block->end->kind == IL_JUMP)
				fprintf(fp, "jmp");
			else
				fprintf(fp, "if");
			fprintf(fp, "\", color = red];");
		}

		if (pair->block->next == NULL && pair->block->next_if == NULL)
		{
			fprintf(fp, "\"");
				debug_basicblock(fp, pair->block);
			fprintf(fp, "\";");
		}
	}

	fprintf(fp, "\n}");
}

void	optimize(void)
{
	t_funcil_pair	*pair;
	bool			updated;

	for (pair = g_func_ils; pair != NULL; pair = pair->next)
	{
		BASICBLOCK_ID = 0;
		g_all_blocks = NULL;

		// init
		g_il = pair->code;
		g_il_last = NULL;

		// int -> intのような意味のないキャストを削除する
		delete_cast_nonsense();

		updated = true;
		while (updated)
		{
			updated = false;
			// pushする以外に副作用がないt_ilの直後にpopがあったら消す
			updated = delete_push_pop();
			// 連続するラベルを1つにまとめる
			updated = delete_label_array() || updated;
		}

		debug_il();

		// 基本ブロックを構成
		g_basicblock = construct_basicblock();

		//基本ブロックから中間表現の列を生成する
		g_il = NULL;
		g_il_last = NULL;
		generate_basicblock_il(g_basicblock);

		// save
		pair->code = g_il;

# ifdef DEBUG
		debug_basicblock_dot(my_strndup(pair->func->name, pair->func->name_len));
# endif
	}
}
