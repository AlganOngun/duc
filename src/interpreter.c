#include "error.h"
#include "ast.h"
#include "interpreter.h"
#include "symbol_table.h"
#include "command.h"

static enum EXT_CODE eval_subcommands(struct ast_node *command_node,
				      struct command_base *cb,
				      struct symbol_table *symbol_table)
{
	for (size_t i = 0; i < command_node->argc; ++i) {
		if (command_node->args[i]->tok.type == TOKEN_SUBCOMMAND) {
			enum EXT_CODE result = eval_subcommands(
				command_node->args[i], cb, symbol_table);
			if (result != EXT_SUCCESS) {
				return result;
			}

			struct error *err = NULL;
			command_exec(cb, command_node->args[i], symbol_table,
				     &err);
			if (err != NULL) {
				error_print(err);
				return EXT_FAIL;
			}
		}
	}
	return EXT_SUCCESS;
}

enum EXT_CODE interpret_ast(const struct ast_node *const ast,
			    struct command_base *cb)
{
	struct scope_table *global_scope = scope_table_create(NULL, 2);
	struct symbol_table *symbol_table = symbol_table_create(global_scope);
	enum EXT_CODE result = EXT_SUCCESS;

	for (size_t i = 0; i < ast->argc; ++i) {
		struct ast_node *current_command = ast->args[i];
		if (current_command->tok.type == TOKEN_EOF) {
			break;
		}
		struct error *err = NULL;

		result = eval_subcommands(current_command, cb, symbol_table);
		if (result != EXT_SUCCESS) {
			symbol_table_free(symbol_table);
			return result;
		}

		command_exec(cb, current_command, symbol_table, &err);
		if (err != NULL) {
			error_print(err);
			symbol_table_free(symbol_table);
			return EXT_FAIL;
		}
	}

	symbol_table_free(symbol_table);
	return EXT_SUCCESS;
}
