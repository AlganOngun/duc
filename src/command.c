#include "symbol_table.h"
#include "ast.h"
#include "command.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CB_SIZE 4

static void resize_cb(struct command_base *cb);
static struct command get_command(struct command_base *cb, const char *name);

struct command_base *command_base_create()
{
	struct command_base *cb = malloc(sizeof(*cb));
	assert(cb != NULL);

	cb->reserved_commands =
		malloc(INITIAL_CB_SIZE * sizeof(struct command));
	assert(cb->reserved_commands != NULL);

	cb->allocated = INITIAL_CB_SIZE;
	cb->length = 0;

	return cb;
}

void command_register(struct command_base *cb, const char *name,
		      command_func func)
{
	if (cb->length >= cb->allocated) {
		resize_cb(cb);
	}

	cb->reserved_commands[cb->length].command_name = name;
	cb->reserved_commands[cb->length].func = func;
	++cb->length;
}

static void resize_cb(struct command_base *cb)
{
	cb->allocated *= 2;
	cb->reserved_commands = realloc(cb->reserved_commands,
					cb->allocated * sizeof(struct command));
	assert(cb->reserved_commands != NULL);
}

void command_exec(struct command_base *cb, struct ast_node *command_node,
		  struct symbol_table *sym_table, struct error **err)
{
	struct command c = get_command(cb, command_node->tok.value);
	c.func(command_node, sym_table, err);
}

static struct command get_command(struct command_base *cb, const char *name)
{
	for (size_t i = 0; i < cb->length; ++i) {
		if (strcmp(cb->reserved_commands[i].command_name, name) == 0) {
			return cb->reserved_commands[i];
		}
	}
	return (struct command){ .command_name = "" };
}

bool command_exists(struct command_base *cb, const char *name)
{
	return strcmp(get_command(cb, name).command_name, "") != 0;
}
