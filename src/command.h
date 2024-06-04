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
	struct command *reserved_commands;
	size_t allocated;
	size_t length;
};

struct command_base *command_base_create();

void command_register(struct command_base *cb, const char *name,
		      command_func func);

void command_exec(struct command_base *cb, struct ast_node *command_node,
		  struct symbol_table *sym_table, struct error **err);

bool command_exists(struct command_base *cb, const char *name);

#endif
