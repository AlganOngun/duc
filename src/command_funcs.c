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

static bool is_literal(struct ast_node *node);
static bool is_identifier(struct ast_node *node);
static const char *parse_sym_val(const struct symbol *sym);
static enum SYMBOL_TYPE tok_type_to_sym_type(const struct token tok);
static union symbol_val tok_to_sym_val(struct symbol_table *sym_table,
				       const struct token tok,
				       struct error **err);
static struct symbol *get_sym(struct symbol_table *sym_table, struct token tok,
			      struct error **err);

void command_func_create(struct ast_node *command_node,
			 struct symbol_table *sym_table, struct error **err)
{
	if (command_node->argc != 2) {
		*err = error_create(
			ERROR_INTERPRETER, ERROR_RUNTIME_ERROR,
			command_node->tok.line, command_node->tok.column,
			"CREATE Command must have two arguments first one being a valid identifier and second one a literal to initialize");
		return;
	}
	if (!is_identifier(command_node->args[0])) {
		*err = error_create(
			ERROR_INTERPRETER, ERROR_RUNTIME_ERROR,
			command_node->tok.line, command_node->tok.column,
			"CREATE Command only accepts a valid identifier as the first argument");
		return;
	}

	if (!is_literal(command_node->args[1])) {
		*err = error_create(
			ERROR_INTERPRETER, ERROR_RUNTIME_ERROR,
			command_node->tok.line, command_node->tok.column,
			"CREATE Command only accepts a valid literal as the second argument (initializer)");
		return;
	}

	const char *identifier = command_node->args[0]->tok.value;
	enum SYMBOL_TYPE sym_type =
		tok_type_to_sym_type(command_node->args[1]->tok);

	struct error *conv_err = NULL;
	union symbol_val sym_val = tok_to_sym_val(
		sym_table, command_node->args[1]->tok, &conv_err);
	if (conv_err != NULL) {
		*err = conv_err;
		return;
	}

	symbol_table_insert(
		sym_table, identifier, sym_type, sym_val,
		(struct symbol_call_data){ command_node->args[0]->tok.line,
					   command_node->args[0]->tok.column },
		err);
}

void command_func_print(struct ast_node *command_node,
			struct symbol_table *sym_table, struct error **err)
{
	if (command_node->argc != 1) {
		*err = error_create(ERROR_INTERPRETER, ERROR_RUNTIME_ERROR,
				    command_node->tok.line,
				    command_node->tok.column,
				    "PRINT Command can only have one argument");
		return;
	}

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
	if (command_node->argc != 2) {
		*err = error_create(
			ERROR_INTERPRETER, ERROR_RUNTIME_ERROR,
			command_node->tok.line, command_node->tok.column,
			"SET Command must have two arguments first one being a valid identifier and second one an identifier or a literal to copy");
		return;
	}
	if (!is_identifier(command_node->args[0])) {
		*err = error_create(
			ERROR_INTERPRETER, ERROR_RUNTIME_ERROR,
			command_node->tok.line, command_node->tok.column,
			"SET Command only accepts a valid identifier as the first argument");
		return;
	}

	if ((!is_literal(command_node->args[1])) &&
	    (!is_identifier(command_node->args[1]))) {
		*err = error_create(
			ERROR_INTERPRETER, ERROR_RUNTIME_ERROR,
			command_node->tok.line, command_node->tok.column,
			"SET Command only accepts a valid literal or a valid identifier as the second argument (value to set)");
		return;
	}

	const char *identifier = command_node->args[0]->tok.value;
	const struct token tok = command_node->tok;
	struct error *conv_err = NULL;
	union symbol_val sym_val = tok_to_sym_val(
		sym_table, command_node->args[1]->tok, &conv_err);
	if (conv_err != NULL) {
		*err = conv_err;
		return;
	}

	symbol_table_change(sym_table, identifier, sym_val,
			    (struct symbol_call_data){ tok.line, tok.column },
			    err);
}

void subcommand_func_add(struct ast_node *command_node,
			 struct symbol_table *sym_table, struct error **err)
{
	printf("ADD Called\n");
}

static struct symbol *get_sym(struct symbol_table *sym_table, struct token tok,
			      struct error **err)
{
	struct symbol *var = symbol_table_find(
		sym_table, tok.value,
		(struct symbol_call_data){ tok.line, tok.column }, err);
	if (var == NULL) {
		return NULL;
	}

	return var;
}

static enum SYMBOL_TYPE tok_type_to_sym_type(const struct token tok)
{
	assert((tok.type == TOKEN_INT) || (tok.type == TOKEN_STR));
	switch (tok.type) {
	case TOKEN_INT:
		return SYMBOL_INT;
	case TOKEN_STR:
		return SYMBOL_STR;
	default: // Assert above covers these cases. Only necessary for warnings to go away
		return SYMBOL_INT;
	}
}

static union symbol_val tok_to_sym_val(struct symbol_table *sym_table,
				       const struct token tok,
				       struct error **err)
{
	assert((tok.type == TOKEN_INT) || (tok.type == TOKEN_STR) ||
	       (tok.type == TOKEN_IDENTIFIER));
	switch (tok.type) {
	case TOKEN_INT:
		intmax_t val = strtoimax(tok.value, NULL, 10);
		if (val == INTMAX_MAX && errno == ERANGE) {
			*err = error_create(
				ERROR_INTERPRETER, ERROR_RUNTIME_ERROR,
				tok.line, tok.column,
				"Integer exceeds the max int limit");
			return (union symbol_val){};
		}
		return (union symbol_val){ .int_val = val };
	case TOKEN_STR:
		return (union symbol_val){ .str_val = tok.value };
	case TOKEN_IDENTIFIER:
		struct symbol *var = get_sym(sym_table, tok, err);
		if (var == NULL) {
			return (union symbol_val){};
		}

		return var->value;
	default: // Assert above covers these cases. Only necessary for warnings to go away
		return (union symbol_val){};
	}
}

static bool is_literal(struct ast_node *node)
{
	switch (node->tok.type) {
	case TOKEN_INT:
		return true;
	case TOKEN_STR:
		return true;
	default:
		return false;
	}
}

static bool is_identifier(struct ast_node *node)
{
	return node->tok.type == TOKEN_IDENTIFIER;
}

static const char *parse_sym_val(const struct symbol *sym)
{
	switch (sym->type) {
	case SYMBOL_INT:
		size_t length = snprintf(NULL, 0, "%d", sym->value.int_val);
		char *str = malloc(length + 1);
		snprintf(str, length + 1, "%d", sym->value.int_val);
		return str;
	case SYMBOL_STR:
		return sym->value.str_val;
	default:
		return "";
	}
}
