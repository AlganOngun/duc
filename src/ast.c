#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ast_node *ast_node_create(struct token tok)
{
	struct ast_node *node = malloc(sizeof(*node));
	if (!node)
		return NULL;

	node->tok = tok;
	node->args = NULL;
	node->argc = 0;

	return node;
}

void ast_add_arg(struct ast_node *node, struct ast_node *arg)
{
	if (!node || !arg)
		return;

	struct ast_node **new_args = realloc(
		node->args, sizeof(struct ast_node *) * (node->argc + 1));
	if (!new_args)
		return;

	node->args = new_args;
	node->args[node->argc++] = arg;
}

static void ast_print_impl(const struct ast_node *const node, int level,
			   bool last)
{
	if (!node)
		return;

	// Print the tree structure
	for (int i = 0; i < level - 1; ++i) {
		printf("│   ");
	}

	if (level > 0) {
		if (last) {
			printf("└── ");
		} else {
			printf("├── ");
		}
	}

	// Print the node value
	printf("%s\n", node->tok.value ? node->tok.value : "NULL");

	// Recursively print children nodes
	for (size_t i = 0; i < node->argc; ++i) {
		ast_print_impl(node->args[i], level + 1, i == node->argc - 1);
	}
}

void ast_print(const struct ast_node *const root)
{
	ast_print_impl(root, 0, true);
}

void ast_free(struct ast_node *node)
{
	if (!node)
		return;

	for (size_t i = 0; i < node->argc; ++i) {
		ast_free(node->args[i]);
	}
	free(node->args);
	free(node);
}
