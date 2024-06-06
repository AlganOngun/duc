#ifndef COMMAND_FUNCS_H
#define COMMAND_FUNCS_H

struct ast_node;
struct symbol_table;
struct error;

void command_func_create(struct ast_node *command_node,
			 struct symbol_table *sym_table, struct error **err);
void command_func_print(struct ast_node *command_node,
			struct symbol_table *sym_table, struct error **err);
void command_func_set(struct ast_node *command_node,
		      struct symbol_table *sym_table, struct error **err);

void subcommand_func_arithmetic(struct ast_node *command_node,
				struct symbol_table *sym_table,
				struct error **err);

#endif
