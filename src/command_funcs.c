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

static char *num_to_str(struct sym_val_data x);
static char *int_to_str(int x);
static char *double_to_str(double x);
static bool is_literal(const struct ast_node *node);
static bool is_identifier(const struct ast_node *node);
static bool is_num(enum SYMBOL_TYPE type);
static const char *parse_sym_val(const struct symbol *sym);
static enum SYMBOL_TYPE tok_to_sym_type(const struct token tok);
static struct sym_val_data tok_to_sym_val(struct symbol_table *sym_table,
					  const struct token tok,
					  struct error **err);
static struct symbol *get_sym(struct symbol_table *sym_table,
			      const struct token tok, struct error **err);
static struct sym_val_data do_arithmetic(struct token tok,
					 const struct sym_val_data x,
					 const struct sym_val_data y);
static int do_int_arithmetic(struct token tok, int x, int y);
static double do_double_arithmetic(struct token tok, double x, double y);

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

	if (is_num(left_val.type)) {
		if (is_num(right_val.type)) {
			struct sym_val_data sum = do_arithmetic(
				command_node->tok, left_val, right_val);
			char *str = num_to_str(sum);
			assert(strcmp(str, "") != 0);

			tok.value = str;
			for (char *c = str; *c != '\0'; ++c) {
				if (*c == '.') {
					tok.type = TOKEN_DOUBLE;
					break;
				}
			}
		}
	}

	command_node->tok = tok;
}

static struct sym_val_data do_arithmetic(struct token tok,
					 const struct sym_val_data x,
					 const struct sym_val_data y)
{
	bool double_arithmetic = false;
	double x_d = 0.0;
	double y_d = 0.0;
	int x_i = 0;
	int y_i = 0;
	if (x.type == SYMBOL_DOUBLE) {
		x_d = x.val.double_val;
		if (y.type == SYMBOL_DOUBLE) {
			y_d = y.val.double_val;
		} else {
			y_d = (double)y.val.int_val;
		}
		double_arithmetic = true;
	} else if (y.type == SYMBOL_DOUBLE) {
		y_d = y.val.double_val;
		x_d = (double)x.val.int_val;
		double_arithmetic = true;
	} else {
		x_i = x.val.int_val;
		y_i = y.val.int_val;
		double_arithmetic = false;
	}

	if (double_arithmetic) {
		double res = do_double_arithmetic(tok, x_d, y_d);
		return (struct sym_val_data){ .type = SYMBOL_DOUBLE,
					      .val = (union symbol_val){
						      .double_val = res } };
	} else {
		int res = do_int_arithmetic(tok, x_i, y_i);
		return (struct sym_val_data){ .type = SYMBOL_INT,
					      .val = (union symbol_val){
						      .int_val = res } };
	}
}

static int do_int_arithmetic(struct token tok, int x, int y)
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
static double do_double_arithmetic(struct token tok, double x, double y)
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
	assert(tok.type == TOKEN_INT || tok.type == TOKEN_STR ||
	       tok.type == TOKEN_DOUBLE);
	switch (tok.type) {
	case TOKEN_INT:
		return SYMBOL_INT;
	case TOKEN_STR:
		return SYMBOL_STR;
	case TOKEN_DOUBLE:
		return SYMBOL_DOUBLE;
	default: // For warnings
		return SYMBOL_STR;
	}
}

static struct sym_val_data tok_to_sym_val(struct symbol_table *sym_table,
					  const struct token tok,
					  struct error **err)
{
	assert(tok.type == TOKEN_INT || tok.type == TOKEN_STR ||
	       tok.type == TOKEN_IDENTIFIER || tok.type == TOKEN_DOUBLE);
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
	if (tok.type == TOKEN_DOUBLE) {
		double val = strtod(tok.value, NULL);
		return (struct sym_val_data){ .type = SYMBOL_DOUBLE,
					      .val = { .double_val = val } };
	}
	struct symbol *var = get_sym(sym_table, tok, err);
	return (var != NULL) ? (struct sym_val_data){ .type = var->type,
						      .val = var->value } :
			       (struct sym_val_data){};
}

static bool is_literal(const struct ast_node *node)
{
	return node->tok.type == TOKEN_INT || node->tok.type == TOKEN_STR ||
	       node->tok.type == TOKEN_DOUBLE;
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
	if (sym->type == SYMBOL_DOUBLE) {
		const char *str = double_to_str(sym->value.double_val);
		assert(strcmp(str, "") != 0);
		return str;
	}
	if (sym->type == SYMBOL_STR) {
		return sym->value.str_val;
	}
	return "";
}

static char *num_to_str(struct sym_val_data x)
{
	assert(x.type == SYMBOL_INT || x.type == SYMBOL_DOUBLE);
	switch (x.type) {
	case SYMBOL_INT:
		return int_to_str(x.val.int_val);
	case SYMBOL_DOUBLE:
		return double_to_str(x.val.double_val);
	default:
		return "";
	}
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

static char *double_to_str(double x)
{
	size_t length = snprintf(NULL, 0, "%f", x);
	char *str = malloc(length + 1);
	if (str == NULL) {
		return "";
	}
	snprintf(str, length + 1, "%f", x);
	return str;
}

static bool is_num(enum SYMBOL_TYPE type)
{
	return type == SYMBOL_INT || type == SYMBOL_DOUBLE;
}
