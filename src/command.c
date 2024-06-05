#include "symbol_table.h"
#include "ast.h"
#include "command.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_COMMANDS_SIZE 4
#define INITIAL_SUBCOMMANDS_SIZE 4

static void resize_commands(struct command_base *cb);
static void resize_subcommands(struct command_base *cb);
static struct command get_command(struct command_base *cb, const char *name);
static struct command get_subcommand(struct command_base *cb, const char *name);

struct command_base *command_base_create()
{
	struct command_base *cb = malloc(sizeof(*cb));
	assert(cb != NULL);

	cb->commands = malloc(INITIAL_COMMANDS_SIZE * sizeof(struct command));
	assert(cb->commands != NULL);

	cb->subcommands =
		malloc(INITIAL_SUBCOMMANDS_SIZE * sizeof(struct command));
	assert(cb->subcommands != NULL);

	cb->commands_allocated = INITIAL_COMMANDS_SIZE;
	cb->commands_length = 0;

	cb->subcommands_allocated = INITIAL_SUBCOMMANDS_SIZE;
	cb->subcommands_length = 0;

	return cb;
}

void command_register(struct command_base *cb, const char *name,
		      command_func func)
{
	if (cb->commands_length >= cb->commands_allocated) {
		resize_commands(cb);
	}

	cb->commands[cb->commands_length].command_name = name;
	cb->commands[cb->commands_length].func = func;
	++cb->commands_length;
}

void subcommand_register(struct command_base *cb, const char *name,
			 command_func func)
{
	if (cb->subcommands_length >= cb->subcommands_allocated) {
		resize_subcommands(cb);
	}

	cb->subcommands[cb->subcommands_length].command_name = name;
	cb->subcommands[cb->subcommands_length].func = func;
	++cb->subcommands_length;
}

static void resize_commands(struct command_base *cb)
{
	cb->commands_allocated *= 2;
	cb->commands = realloc(cb->commands,
			       cb->commands_allocated * sizeof(struct command));
	assert(cb->commands != NULL);
}

static void resize_subcommands(struct command_base *cb)
{
	cb->subcommands_allocated *= 2;
	cb->subcommands =
		realloc(cb->subcommands,
			cb->subcommands_allocated * sizeof(struct command));
	assert(cb->subcommands != NULL);
}

void command_exec(struct command_base *cb, struct ast_node *command_node,
		  struct symbol_table *sym_table, struct error **err)
{
	struct command c = get_command(cb, command_node->tok.value);
	c.func(command_node, sym_table, err);
}

static struct command get_command(struct command_base *cb, const char *name)
{
	for (size_t i = 0; i < cb->commands_length; ++i) {
		if (strcmp(cb->commands[i].command_name, name) == 0) {
			return cb->commands[i];
		}
	}
	return (struct command){ .command_name = "" };
}

static struct command get_subcommand(struct command_base *cb, const char *name)
{
	for (size_t i = 0; i < cb->subcommands_length; ++i) {
		if (strcmp(cb->subcommands[i].command_name, name) == 0) {
			return cb->subcommands[i];
		}
	}
	return (struct command){ .command_name = "" };
}

bool command_exists(struct command_base *cb, const char *name)
{
	return strcmp(get_command(cb, name).command_name, "") != 0;
}

bool subcommand_exists(struct command_base *cb, const char *name)
{
	return strcmp(get_subcommand(cb, name).command_name, "") != 0;
}
