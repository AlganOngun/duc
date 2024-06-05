#include "error.h"
#include "lexer.h"
#include "command.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>

static struct token create_command_or_identifier_token(struct lexer *lexer);
static struct token create_int(struct lexer *lexer, struct error **err);
static struct token create_str(struct lexer *lexer, struct error **err);
static void skip_comment(struct lexer *lexer);

struct lexer *lexer_create(const char *source, struct command_base *cb)
{
	struct lexer *lexer = malloc(sizeof(*lexer));
	assert(lexer != NULL);

	lexer->source = strdup(source);
	if (!lexer->source) {
		free(lexer);
		return NULL;
	}

	lexer->pos = 0;
	lexer->line = 1;
	lexer->column = 1;
	lexer->current_char = lexer->source[0];
	lexer->cb = cb;

	return lexer;
}

void lexer_advance(struct lexer *lexer)
{
	if (lexer->current_char == '\n') {
		lexer->line++;
		lexer->column = 1;
	} else {
		lexer->column++;
	}

	++lexer->pos;
	if (lexer->pos >= strlen(lexer->source)) {
		lexer->current_char = '\0';
	} else {
		lexer->current_char = lexer->source[lexer->pos];
	}
}

static void skip_comment(struct lexer *lexer)
{
	while (lexer->current_char != '\n' && lexer->current_char != '\0') {
		lexer_advance(lexer);
	}
	lexer_advance(lexer); // To move past the newline character
}

struct token lexer_next_token(struct lexer *lexer, struct error **err)
{
	while (lexer->current_char != '\0') {
		if (lexer->current_char == '#') {
			skip_comment(lexer);
			continue;
		}

		if (isdigit(lexer->current_char)) {
			return create_int(lexer, err);
		}
		if (lexer->current_char == '|') {
			return create_str(lexer, err);
		}
		if (isalpha(lexer->current_char)) {
			return create_command_or_identifier_token(lexer);
		}
		if (isspace(lexer->current_char)) {
			lexer_advance(lexer);
			continue;
		}
		lexer_advance(lexer);
	}

	return (struct token){ .type = TOKEN_EOF, .value = NULL };
}

static struct token create_command_or_identifier_token(struct lexer *lexer)
{
	size_t start_pos = lexer->pos;

	while (isalnum(lexer->current_char) || lexer->current_char == '_') {
		lexer_advance(lexer);
	}

	size_t len = lexer->pos - start_pos;

	char *value = malloc(len + 1);
	assert(value != NULL);

	strncpy(value, lexer->source + start_pos, len);
	value[len] = '\0';

	if (command_exists(lexer->cb, value)) {
		return (struct token){ .type = TOKEN_COMMAND,
				       .value = value,
				       .line = lexer->line,
				       .column = lexer->column - len };
	} else if (subcommand_exists(lexer->cb, value)) {
		return (struct token){ .type = TOKEN_SUBCOMMAND,
				       .value = value,
				       .line = lexer->line,
				       .column = lexer->column - len };
	} else {
		return (struct token){ .type = TOKEN_IDENTIFIER,
				       .value = value,
				       .line = lexer->line,
				       .column = lexer->column - len };
	}
}

static struct token create_int(struct lexer *lexer, struct error **err)
{
	size_t start_pos = lexer->pos;
	while (isdigit(lexer->source[lexer->pos])) {
		lexer_advance(lexer);
	}
	if (isalpha(lexer->source[lexer->pos])) {
		*err = error_create(ERROR_LEXER, ERROR_INVALID_IDENTIFIER,
				    lexer->line, lexer->column,
				    "Invalid identifier starting with a digit");

		return (struct token){
			.type = TOKEN_INVALID_IDENTIFIER,
			.value = "",
			.line = lexer->line,
			.column = lexer->column,
		};
	}

	size_t len = lexer->pos - start_pos;

	char *value = malloc(len + 1);
	assert(value != NULL);

	strncpy(value, lexer->source + start_pos, len);
	value[len] = '\0';

	return (struct token){ .type = TOKEN_INT,
			       .value = value,
			       .line = lexer->line,
			       .column = lexer->column - len };
}

static struct token create_str(struct lexer *lexer, struct error **err)
{
	lexer_advance(lexer); //Skip the literal symbol "|"
	size_t start_pos = lexer->pos;
	while (lexer->source[lexer->pos] != '|') {
		if (lexer->source[lexer->pos] == '\0') {
			*err = error_create(
				ERROR_LEXER, ERROR_UNEXPECTED_EOF, lexer->line,
				lexer->column,
				"Unexpected end of file while scanning string literal");
			return (struct token){ .type = TOKEN_EOF,
					       .value = NULL };
		}
		lexer_advance(lexer);
	}

	size_t len = lexer->pos - start_pos;

	char *value = malloc(len + 1);
	assert(value != NULL);

	strncpy(value, lexer->source + start_pos, len);
	value[len] = '\0';

	lexer_advance(lexer); //Skip the ending literal symbol
	return (struct token){ .type = TOKEN_STR,
			       .value = value,
			       .line = lexer->line,
			       .column = lexer->column - len };
}

void lexer_destroy(struct lexer *lexer)
{
	if (lexer) {
		free((char *)lexer->source);
		free(lexer);
	}
}

void token_destroy(struct token *token)
{
	if (token && token->value) {
		free(token->value);
		token->value = NULL;
	}
}
