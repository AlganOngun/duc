#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

struct error *error_create(enum ERROR_TYPE type, enum ERROR_CODE code,
			   size_t line, size_t column, const char *format, ...)
{
	struct error *err;
	va_list args;
	size_t len;

	err = malloc(sizeof(*err));
	if (!err)
		return NULL;

	err->type = type;
	err->code = code;
	err->line = line;
	err->column = column;

	va_start(args, format);
	len = vsnprintf(NULL, 0, format, args) + 1;
	va_end(args);

	err->message = malloc(len);
	if (!err->message) {
		free(err);
		return NULL;
	}

	va_start(args, format);
	vsnprintf((char *)err->message, len, format, args);
	va_end(args);

	return err;
}

void error_print(const struct error *err)
{
	const char *type_str;

	if (!err)
		return;

	switch (err->type) {
	case ERROR_LEXER:
		type_str = "Lexer Error";
		break;
	case ERROR_PARSER:
		type_str = "Parser Error";
		break;
	case ERROR_INTERPRETER:
		type_str = "Interpreter Error";
		break;
	default:
		type_str = "Unknown Error";
		break;
	}

	fprintf(stderr, "%s at line %zu, column %zu: %s\n", type_str, err->line,
		err->column, err->message);
}

void error_free(struct error *err)
{
	if (err) {
		free((char *)err->message);
		free(err);
	}
}
