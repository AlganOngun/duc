#ifndef ERROR_H
#define ERROR_H

#include <stddef.h>

enum ERROR_TYPE { ERROR_LEXER, ERROR_PARSER, ERROR_INTERPRETER };

enum ERROR_CODE {
	ERROR_INVALID_IDENTIFIER,
	ERROR_UNEXPECTED_EOF,
	ERROR_SYNTAX_ERROR,
	ERROR_RUNTIME_ERROR,
	ERROR_DUPLICATE_IDENTIFIER,
};

struct error {
	enum ERROR_TYPE type;
	enum ERROR_CODE code;
	size_t line;
	size_t column;
	const char *message;
};

struct error *error_create(enum ERROR_TYPE type, enum ERROR_CODE code,
			   size_t line, size_t column, const char *format, ...);

void error_print(const struct error *err);

void error_free(struct error *err);

#endif
