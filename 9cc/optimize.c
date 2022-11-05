#include "il.h"
#include "optimize.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern t_il	*g_il;

static t_basicblock		*g_basicblock;
static t_il				*g_il_last;

static t_basicblock	*create_basicblock(t_il *start, t_il *end, t_basicblock *next, t_basicblock *next_if)
{
	t_basicblock	*block;

	block = calloc(1, sizeof(t_basicblock));
	block->start = start;
	block->end = end;
	block->next = next;
	block->next = next_if;
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

static t_basicblock	*pop_ilblock_pair(t_pair_ilblock **pairs)
{
	t_basicblock	*block;
	t_pair_ilblock	*ptr;

	if (*pairs == NULL)
		return (NULL);

	ptr = *(pairs);
	block = ptr->block;
	*pairs = ptr->next;
	free(ptr);
	return (block);
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
		connect_basicblock_recursively(pairs, target_block);
	}

	if (block->end->kind == IL_JUMP
	|| block->end->kind == IL_JUMP_TRUE
	|| block->end->kind == IL_JUMP_FALSE)
	{
		target_block = search_ilblock_with_label(pairs, block->end->label_str);
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
	t_pair_ilblock	*pair;
	t_basicblock	*headblock;

	block			= NULL;
	block_now		= NULL;
	code			= NULL;
	il_leader_pairs	= NULL;
	headblock		= NULL;

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
		}
		if (block_now != NULL)
			block_now->end = code;
	}

	for (pair = il_leader_pairs; pair != NULL; pair = pair->next)
	{
		pair->block->start->before = NULL;
		pair->block->end->next = NULL;
	}

	// 基本ブロックをつなぐ
	headblock = search_ilblock_pair(il_leader_pairs, g_il);
	connect_basicblock_recursively(il_leader_pairs, headblock);

	return headblock;
}

// とりあえずはnextを生成するだけ生成して、ifはスタック
static void	generate_basicblock_il(t_basicblock *block)
{
	if (block == NULL || block->il_generated)
		return ;
	block->il_generated = true;

	printf("# New Block %d -> %d\n",
			block->start->ilid_unique, block->end->ilid_unique);

	if (g_il == NULL)
	{
		g_il = block->start;
		g_il_last = block->end;
	}
	else
	{
		g_il_last->next = block->start;
		g_il_last = block->end;
	}

	g_il_last->next = NULL;
	printf("#next\n");
	generate_basicblock_il(block->next);
	printf("#next_if\n");
	generate_basicblock_il(block->next_if);
	printf("#end\n");
}

void	optimize(void)
{
	g_basicblock = construct_basicblock();

	g_il = NULL;
	g_il_last = NULL;

	//基本ブロックから中間表現の列を生成する
	generate_basicblock_il(g_basicblock);
}
