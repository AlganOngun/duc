#ifndef LEXER_H
#define LEXER_H

#include <stdlib.h>

struct command_base;
struct error;

enum TOKEN_TYPE {
	TOKEN_START,
	TOKEN_COMMAND,
	TOKEN_SUBCOMMAND,
	TOKEN_IDENTIFIER,
	TOKEN_INT,
	TOKEN_STR,
	TOKEN_INVALID_IDENTIFIER,
	TOKEN_EOF,
};

struct token {
	enum TOKEN_TYPE type;
	size_t line;
	size_t column;
	size_t max_argc;
	char *value;
};

struct lexer {
	const char *source;
	size_t pos;
	size_t line;
	size_t column;
	char current_char;

	struct command_base *cb;
};

struct lexer *lexer_create(const char *source, struct command_base *cb);
struct token lexer_next_token(struct lexer *lexer, struct error **err);

void lexer_destroy(struct lexer *lexer);
void token_destroy(struct token *token);

#endif
