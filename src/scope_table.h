#ifndef SCOPE_TABLE_H
#define SCOPE_TABLE_H

#include <stddef.h>

struct error;

enum SYMBOL_TYPE {
	SYMBOL_INT,
	SYMBOL_STR,
};

union symbol_val {
	int int_val;
	char *str_val;
};

struct symbol {
	char *identifier;
	enum SYMBOL_TYPE type;
	union symbol_val value;
	struct symbol *next;
	size_t line;
	size_t column;
};

struct symbol_call_data {
	size_t line;
	size_t column;
};

struct scope_table {
	struct symbol **symbols;
	struct scope_table *parent;
	size_t size;
	size_t count;
};

struct scope_table *scope_table_create(struct scope_table *parent, size_t size);
void scope_table_free(struct scope_table *table);
void scope_table_insert(struct scope_table *table, const char *identifier,
			enum SYMBOL_TYPE type, union symbol_val value,
			struct symbol_call_data call_data, struct error **err);
struct symbol *scope_table_find(struct scope_table *table,
				const char *identifier,
				struct symbol_call_data call_data,
				struct error **err);
void scope_table_delete(struct scope_table *table, const char *identifier);
void scope_table_print(struct scope_table *table);

#endif
