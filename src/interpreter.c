#include "error.h"
#include "ast.h"
#include "interpreter.h"
#include "symbol_table.h"
#include "command.h"

enum EXT_CODE interpret_ast(const struct ast_node *const ast,
			    struct command_base *cb)
{
	struct scope_table *global_scope = scope_table_create(NULL, 2);
	struct symbol_table *symbol_table = symbol_table_create(global_scope);

	size_t current_command_i = 0;
	struct ast_node *current_command = ast->args[current_command_i];

	while (current_command->tok.type == TOKEN_COMMAND) {
		struct error *err = NULL;
		command_exec(cb, current_command, symbol_table, &err);

		if (err != NULL) {
			error_print(err);
			return EXT_FAIL;
		}

		++current_command_i;
		current_command = ast->args[current_command_i];
	}

	return EXT_SUCCESS;
}
