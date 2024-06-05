#ifndef COMMAND_H
#define COMMAND_H

#include <stdlib.h>

struct ast_node;
struct error;
struct symbol_table;

typedef void (*command_func)(struct ast_node *command_node,
			     struct symbol_table *sym_table,
			     struct error **err);

struct command {
	const char *command_name;
	command_func func;
};

// List to store reserved commands
struct command_base {
	struct command *commands;
	struct command *subcommands;
	size_t commands_allocated;
	size_t commands_length;
	size_t subcommands_allocated;
	size_t subcommands_length;
};

struct command_base *command_base_create();

void command_register(struct command_base *cb, const char *name,
		      command_func func);
void subcommand_register(struct command_base *cb, const char *name,
			 command_func func);

void command_exec(struct command_base *cb, struct ast_node *command_node,
		  struct symbol_table *sym_table, struct error **err);

bool command_exists(struct command_base *cb, const char *name);
bool subcommand_exists(struct command_base *cb, const char *name);

#endif
