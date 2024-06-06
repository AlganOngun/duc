#include "ast.h"
#include "error.h"
#include "symbol_table.h"
#include "command_funcs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>

struct sym_val_data {
	enum SYMBOL_TYPE type;
	union symbol_val val;
};

static char *int_to_str(int x);
static bool is_literal(const struct ast_node *node);
static bool is_identifier(const struct ast_node *node);
static const char *parse_sym_val(const struct symbol *sym);
static enum SYMBOL_TYPE tok_to_sym_type(const struct token tok);
static struct sym_val_data tok_to_sym_val(struct symbol_table *sym_table,
					  const struct token tok,
					  struct error **err);
static struct symbol *get_sym(struct symbol_table *sym_table,
			      const struct token tok, struct error **err);
static int do_arithmetic(struct token tok, int x, int y);

void command_func_create(struct ast_node *command_node,
			 struct symbol_table *sym_table, struct error **err)
{
	const char *identifier = command_node->args[0]->tok.value;
	enum SYMBOL_TYPE sym_type = tok_to_sym_type(command_node->args[1]->tok);

	struct sym_val_data sym_val =
		tok_to_sym_val(sym_table, command_node->args[1]->tok, err);
	if (*err != NULL) {
		return;
	}

	symbol_table_insert(
		sym_table, identifier, sym_type, sym_val.val,
		(struct symbol_call_data){ command_node->args[0]->tok.line,
					   command_node->args[0]->tok.column },
		err);
}

void command_func_print(struct ast_node *command_node,
			struct symbol_table *sym_table, struct error **err)
{
	if (is_literal(command_node->args[0])) {
		printf("%s\n", command_node->args[0]->tok.value);
	} else if (is_identifier(command_node->args[0])) {
		struct symbol *var =
			get_sym(sym_table, command_node->args[0]->tok, err);
		if (var == NULL) {
			return;
		}

		printf("%s\n", parse_sym_val(var));
	}
}

void command_func_set(struct ast_node *command_node,
		      struct symbol_table *sym_table, struct error **err)
{
	const char *identifier = command_node->args[0]->tok.value;

	struct sym_val_data sym_val =
		tok_to_sym_val(sym_table, command_node->args[1]->tok, err);
	if (*err != NULL) {
		return;
	}

	symbol_table_change(
		sym_table, identifier, sym_val.val,
		(struct symbol_call_data){ command_node->tok.line,
					   command_node->tok.column },
		err);
}

void subcommand_func_arithmetic(struct ast_node *command_node,
				struct symbol_table *sym_table,
				struct error **err)
{
	struct token tok = {
		.type = TOKEN_INT,
		.line = command_node->tok.line,
		.column = command_node->tok.column,
	};

	struct token left = command_node->args[0]->tok;
	struct token right = command_node->args[1]->tok;
	struct sym_val_data left_val = tok_to_sym_val(sym_table, left, err);
	struct sym_val_data right_val = tok_to_sym_val(sym_table, right, err);
	if (*err != NULL) {
		return;
	}

	if (left_val.type == SYMBOL_INT) {
		if (right_val.type == SYMBOL_INT) {
			int sum = do_arithmetic(command_node->tok,
						left_val.val.int_val,
						right_val.val.int_val);
			char *str = int_to_str(sum);
			assert(strcmp(str, "") != 0);

			tok.value = str;
		}
	}

	struct ast_node *parent = command_node->parent;
	ast_delete_node(command_node);
	command_node = ast_node_create(tok);
	ast_add_arg(parent, command_node);
}

static int do_arithmetic(struct token tok, int x, int y)
{
	if (strcmp(tok.value, "ADD") == 0) {
		return x + y;
	}
	if (strcmp(tok.value, "SUB") == 0) {
		return x - y;
	}
	if (strcmp(tok.value, "MUL") == 0) {
		return x * y;
	}
	if (strcmp(tok.value, "DIV") == 0) {
		return x / y;
	}
	return -1;
}

static struct symbol *get_sym(struct symbol_table *sym_table,
			      const struct token tok, struct error **err)
{
	return symbol_table_find(
		sym_table, tok.value,
		(struct symbol_call_data){ tok.line, tok.column }, err);
}

static enum SYMBOL_TYPE tok_to_sym_type(const struct token tok)
{
	assert(tok.type == TOKEN_INT || tok.type == TOKEN_STR);
	return (tok.type == TOKEN_INT) ? SYMBOL_INT : SYMBOL_STR;
}

static struct sym_val_data tok_to_sym_val(struct symbol_table *sym_table,
					  const struct token tok,
					  struct error **err)
{
	assert(tok.type == TOKEN_INT || tok.type == TOKEN_STR ||
	       tok.type == TOKEN_IDENTIFIER);
	if (tok.type == TOKEN_INT) {
		intmax_t val = strtoimax(tok.value, NULL, 10);
		if (val == INTMAX_MAX && errno == ERANGE) {
			*err = error_create(
				ERROR_INTERPRETER, ERROR_RUNTIME_ERROR,
				tok.line, tok.column,
				"Integer exceeds the max int limit");
			return (struct sym_val_data){};
		}
		return (struct sym_val_data){ .type = SYMBOL_INT,
					      .val = { .int_val = val } };
	}
	if (tok.type == TOKEN_STR) {
		return (struct sym_val_data){ .type = SYMBOL_STR,
					      .val = { .str_val = tok.value } };
	}
	struct symbol *var = get_sym(sym_table, tok, err);
	return (var != NULL) ? (struct sym_val_data){ .type = var->type,
						      .val = var->value } :
			       (struct sym_val_data){};
}

static bool is_literal(const struct ast_node *node)
{
	return node->tok.type == TOKEN_INT || node->tok.type == TOKEN_STR;
}

static bool is_identifier(const struct ast_node *node)
{
	return node->tok.type == TOKEN_IDENTIFIER;
}

static const char *parse_sym_val(const struct symbol *sym)
{
	if (sym->type == SYMBOL_INT) {
		const char *str = int_to_str(sym->value.int_val);
		assert(strcmp(str, "") != 0);
		return str;
	}
	if (sym->type == SYMBOL_STR) {
		return sym->value.str_val;
	}
	return "";
}

static char *int_to_str(int x)
{
	size_t length = snprintf(NULL, 0, "%d", x);
	char *str = malloc(length + 1);
	if (str == NULL) {
		return "";
	}
	snprintf(str, length + 1, "%d", x);
	return str;
}
