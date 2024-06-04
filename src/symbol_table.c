#include "symbol_table.h"
#include "scope_table.h"
#include <stdlib.h>
#include <stdio.h>
#include "error.h"
#include <assert.h>

// Function to create and initialize the symbol table
struct symbol_table *symbol_table_create(struct scope_table *global_scope)
{
	struct symbol_table *table = malloc(sizeof(*table));
	assert(table != NULL);

	table->scopes = malloc(sizeof(struct scope_table *));
	assert(table->scopes != NULL);

	table->scopes[0] = global_scope;
	table->current_scope = global_scope;
	table->allocated = 1;
	table->length = 1;

	return table;
}

// Function to resize the scopes stack
static void resize_stack(struct symbol_table *table)
{
	size_t new_allocated = table->allocated * 2;
	struct scope_table **new_stack = realloc(
		table->scopes, sizeof(struct scope_table *) * new_allocated);
	assert(new_stack != NULL);

	table->scopes = new_stack;
	table->allocated = new_allocated;
}

// Function to push a new scope onto the symbol table stack
void symbol_table_push_scope(struct symbol_table *table,
			     struct scope_table *scope)
{
	if (table->length >= table->allocated) {
		resize_stack(table);
	}

	table->scopes[table->length] = scope;
	table->current_scope = scope;
	table->length++;
}

// Function to pop the current scope from the symbol table stack
void symbol_table_pop_scope(struct symbol_table *table)
{
	if (table->length <= 1) {
		return; // Prevent popping the global scope
	}

	scope_table_free(table->current_scope);
	table->length--;
	table->current_scope = table->scopes[table->length - 1];
}

// Function to find a symbol in the symbol table
struct symbol *symbol_table_find(const struct symbol_table *table,
				 const char *identifier,
				 struct symbol_call_data call_data,
				 struct error **err)
{
	struct scope_table *active_scope = table->current_scope;

	struct symbol *found_symbol =
		scope_table_find(active_scope, identifier, call_data, err);

	while (found_symbol == NULL) {
		if (active_scope->parent == NULL) {
			*err = error_create(
				ERROR_INTERPRETER, ERROR_INVALID_IDENTIFIER,
				call_data.line, call_data.column,
				"No variable found with identifier '%s'",
				identifier);
			return NULL;
		}
		active_scope = active_scope->parent;
	}

	return found_symbol;
}

void symbol_table_change(struct symbol_table *table, const char *identifier,
			 union symbol_val new_value,
			 struct symbol_call_data call_data, struct error **err)
{
	struct symbol *sym =
		symbol_table_find(table, identifier, call_data, err);

	if (sym == NULL) {
		return;
	}

	sym->value = new_value;
}

void symbol_table_insert(struct symbol_table *table, const char *identifier,
			 enum SYMBOL_TYPE type, union symbol_val value,
			 struct symbol_call_data call_data, struct error **err)
{
	scope_table_insert(table->current_scope, identifier, type, value,
			   call_data, err);
}

void symbol_table_delete(struct symbol_table *table, const char *identifier)
{
	scope_table_delete(table->current_scope, identifier);
}

// Function to free all memory associated with the symbol table
void symbol_table_free(struct symbol_table *table)
{
	for (size_t i = 0; i < table->length; i++) {
		scope_table_free(table->scopes[i]);
	}

	free(table->scopes);
	free(table);
}

void symbol_table_print(const struct symbol_table *table)
{
	printf("Symbol Table:\n");
	for (size_t i = 0; i < table->length; i++) {
		printf("Scope %zu:\n", i);
		scope_table_print(table->scopes[i]);
	}
}
