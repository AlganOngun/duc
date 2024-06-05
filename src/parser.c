#include "ast.h"
#include "lexer.h"
#include "error.h"

struct ast_node *parse_command_or_subcommand(struct lexer *lexer,
					     struct token *current_token,
					     struct error **err)
{
	struct ast_node *node = ast_node_create(*current_token);

	struct token arg_tok = lexer_next_token(lexer, err);
	if (*err != NULL) {
		error_print(*err);
		return NULL;
	}

	while (arg_tok.type != TOKEN_COMMAND && arg_tok.type != TOKEN_EOF) {
		struct ast_node *arg;
		if (arg_tok.type == TOKEN_SUBCOMMAND) {
			arg = parse_command_or_subcommand(lexer, &arg_tok, err);
		} else {
			arg = ast_node_create(arg_tok);
		}
		if (*err != NULL) {
			return NULL;
		}
		ast_add_arg(node, arg);

		arg_tok = lexer_next_token(lexer, err);
		if (*err != NULL) {
			error_print(*err);
			return NULL;
		}
	}

	*current_token =
		arg_tok; // Update the current token to the last token read
	return node;
}

struct ast_node *parse_tokens(struct lexer *lexer)
{
	struct ast_node *root = ast_node_create(
		(struct token){ .type = TOKEN_COMMAND, .value = "PROG" });

	struct error *err = NULL;
	struct token token = lexer_next_token(lexer, &err);
	if (err != NULL) {
		error_print(err);
		return NULL;
	}

	while (token.type != TOKEN_EOF) {
		if (token.type == TOKEN_COMMAND) {
			struct ast_node *node = parse_command_or_subcommand(
				lexer, &token, &err);
			if (err != NULL) {
				return NULL;
			}
			ast_add_arg(root, node);
		} else {
			token = lexer_next_token(lexer, &err);
			if (err != NULL) {
				error_print(err);
				return NULL;
			}
		}
	}

	struct ast_node *eof_node = ast_node_create(
		(struct token){ .type = TOKEN_EOF, .value = NULL });
	ast_add_arg(root, eof_node);

	return root;
}
