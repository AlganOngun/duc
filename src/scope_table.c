#include "error.h"
#include "scope_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define INITIAL_SIZE 8
#define LOAD_FACTOR_THRESHOLD 0.75

static void resize_table(struct scope_table *table);

static unsigned int hash_function(const char *key, size_t size)
{
	unsigned long int value = 0;
	size_t i = 0;
	size_t key_len = strlen(key);

	for (; i < key_len; ++i)
		value = value * 37 + key[i];

	return value % size;
}

static struct symbol *create_symbol(const char *identifier,
				    enum SYMBOL_TYPE type,
				    union symbol_val value, size_t line,
				    size_t column)
{
	struct symbol *new_symbol = malloc(sizeof(struct symbol));
	assert(new_symbol != NULL);

	new_symbol->identifier = strdup(identifier);
	assert(new_symbol->identifier != NULL);

	new_symbol->type = type;
	new_symbol->value = value;
	new_symbol->line = line;
	new_symbol->column = column;
	new_symbol->next = NULL;
	return new_symbol;
}

struct scope_table *scope_table_create(struct scope_table *parent, size_t size)
{
	struct scope_table *table = malloc(sizeof(struct scope_table));
	assert(table != NULL);

	table->parent = parent;
	table->size = size;
	table->count = 0;

	table->symbols = calloc(table->size, sizeof(struct symbol *));
	assert(table->symbols != NULL);

	return table;
}

static void free_symbol(struct symbol *symbol)
{
	free(symbol->identifier);
	free(symbol);
}

void scope_table_insert(struct scope_table *table, const char *identifier,
			enum SYMBOL_TYPE type, union symbol_val value,
			struct symbol_call_data call_data, struct error **err)
{
	if ((float)table->count / table->size > LOAD_FACTOR_THRESHOLD) {
		resize_table(table);
	}

	// Check for duplicates in the current scope
	unsigned int index = hash_function(identifier, table->size);
	struct symbol *existing_symbol = table->symbols[index];
	while (existing_symbol) {
		if (strcmp(existing_symbol->identifier, identifier) == 0) {
			*err = error_create(
				ERROR_INTERPRETER, ERROR_DUPLICATE_IDENTIFIER,
				call_data.line, call_data.column,
				"Found a variable with same identifier '%s', this identifier was first used in line: %d, column: %d",
				identifier, existing_symbol->line,
				existing_symbol->column);
			return;
		}
		existing_symbol = existing_symbol->next;
	}

	// Check for duplicates in parent scopes
	struct scope_table *parent = table->parent;
	while (parent) {
		unsigned int parent_index =
			hash_function(identifier, parent->size);
		struct symbol *parent_symbol = parent->symbols[parent_index];
		while (parent_symbol) {
			if (strcmp(parent_symbol->identifier, identifier) ==
			    0) {
				*err = error_create(
					ERROR_INTERPRETER,
					ERROR_DUPLICATE_IDENTIFIER,
					call_data.line, call_data.column,
					"Found a variable with same identifier '%s', this identifier was first used in line: %d, column: %d",
					identifier, existing_symbol->line,
					existing_symbol->column);
			}
			parent_symbol = parent_symbol->next;
		}
		parent = parent->parent;
	}

	struct symbol *new_symbol = create_symbol(
		identifier, type, value, call_data.line, call_data.column);

	new_symbol->next = table->symbols[index];
	table->symbols[index] = new_symbol;
	table->count++;
}

struct symbol *scope_table_find(struct scope_table *table,
				const char *identifier,
				struct symbol_call_data call_data,
				struct error **err)
{
	unsigned int index = hash_function(identifier, table->size);
	struct symbol *symbol = table->symbols[index];
	while (symbol) {
		if (strcmp(symbol->identifier, identifier) == 0) {
			return symbol;
		}
		symbol = symbol->next;
	}
	*err = error_create(ERROR_INTERPRETER, ERROR_INVALID_IDENTIFIER,
			    call_data.line, call_data.column,
			    "Variable with identifier '%s' does not exist",
			    identifier);
	return NULL;
}

void scope_table_delete(struct scope_table *table, const char *identifier)
{
	unsigned int index = hash_function(identifier, table->size);
	struct symbol *symbol = table->symbols[index];
	struct symbol *prev = NULL;
	while (symbol) {
		if (strcmp(symbol->identifier, identifier) == 0) {
			if (prev) {
				prev->next = symbol->next;
			} else {
				table->symbols[index] = symbol->next;
			}
			free_symbol(symbol);
			table->count--;
			return;
		}
		prev = symbol;
		symbol = symbol->next;
	}
}

void scope_table_free(struct scope_table *table)
{
	for (size_t i = 0; i < table->size; i++) {
		struct symbol *symbol = table->symbols[i];
		while (symbol) {
			struct symbol *temp = symbol;
			symbol = symbol->next;
			free_symbol(temp);
		}
	}
	free(table->symbols);
	free(table);
}

void scope_table_print(struct scope_table *table)
{
	printf("Scope Table Contents:\n");
	for (size_t i = 0; i < table->size; i++) {
		struct symbol *node = table->symbols[i];
		if (node) {
			printf("Bucket %zu:\n", i);
			while (node) {
				if (node->type == SYMBOL_INT) {
					printf("  Key: %s, Value: %d\n",
					       node->identifier,
					       node->value.int_val);
				}
				if (node->type == SYMBOL_STR) {
					printf("  Key: %s, Value: %s\n",
					       node->identifier,
					       node->value.str_val);
				}
				node = node->next;
			}
		}
	}
}

static void resize_table(struct scope_table *table)
{
	size_t new_size = table->size * 2;
	struct scope_table *new_table =
		scope_table_create(table->parent, new_size);

	for (size_t i = 0; i < table->size; i++) {
		struct symbol *symbol = table->symbols[i];
		while (symbol) {
			scope_table_insert(new_table, symbol->identifier,
					   symbol->type, symbol->value,
					   (struct symbol_call_data){ 0, 0 },
					   NULL);
			struct symbol *temp = symbol;
			symbol = symbol->next;
			free_symbol(temp);
		}
	}

	free(table->symbols);
	table->symbols = new_table->symbols;
	table->size = new_table->size;
	free(new_table);
}
