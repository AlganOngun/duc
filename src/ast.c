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
	node->parent = NULL;

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
	node->args[node->argc] = arg;
	++node->argc;
	arg->parent = node;
}

void ast_delete_node(struct ast_node *node)
{
	if (!node || !node->parent)
		return;

	struct ast_node *parent = node->parent;
	size_t i;
	for (i = 0; i < parent->argc; ++i) {
		if (parent->args[i] == node) {
			break;
		}
	}

	if (i == parent->argc)
		return; // Node not found in parent args

	for (size_t j = i; j < parent->argc - 1; ++j) {
		parent->args[j] = parent->args[j + 1];
	}

	parent->argc--;
	if (parent->argc == 0) {
		free(parent->args);
		parent->args = NULL;
	} else {
		struct ast_node **new_args = realloc(
			parent->args, sizeof(struct ast_node *) * parent->argc);
		if (new_args)
			parent->args = new_args;
	}

	ast_free(node);
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
