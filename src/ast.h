#ifndef AST_H
#define AST_H

#include "lexer.h"

struct ast_node {
	struct token tok;
	struct ast_node *parent;
	struct ast_node **args;
	size_t argc;
};

struct ast_node *ast_node_create(struct token tok);
void ast_delete_node(struct ast_node *node);
void ast_add_arg(struct ast_node *node, struct ast_node *arg);
void ast_print(const struct ast_node *const node);
void ast_free(struct ast_node *node);

#endif
