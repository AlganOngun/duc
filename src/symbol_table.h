#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdlib.h>
#include "scope_table.h"

struct symbol_table {
	struct scope_table **scopes;
	struct scope_table *current_scope;
	size_t allocated; // Allocated space to use in stack
	size_t length; // Count of scopes in stack
};

struct symbol_table *symbol_table_create(struct scope_table *global_scope);

void symbol_table_push_scope(struct symbol_table *table,
			     struct scope_table *scope);
void symbol_table_pop_scope(struct symbol_table *table);
struct symbol *symbol_table_find(const struct symbol_table *table,
				 const char *identifier,
				 struct symbol_call_data call_data,
				 struct error **err);
void symbol_table_change(struct symbol_table *table, const char *identifier,
			 union symbol_val new_value,
			 struct symbol_call_data call_data, struct error **err);
void symbol_table_insert(struct symbol_table *table, const char *identifier,
			 enum SYMBOL_TYPE type, union symbol_val value,
			 struct symbol_call_data call_data, struct error **err);
void symbol_table_delete(struct symbol_table *table, const char *identifier);

void symbol_table_free(struct symbol_table *table);
void symbol_table_print(const struct symbol_table *table);

#endif
