#include "ast.h"
#include "lexer.h"
#include "error.h"

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
			struct ast_node *node = ast_node_create(token);
			ast_add_arg(root, node);

			struct token arg_tok = lexer_next_token(lexer, &err);
			if (err != NULL) {
				error_print(err);
				return NULL;
			}

			while (arg_tok.type != TOKEN_COMMAND &&
			       arg_tok.type != TOKEN_EOF) {
				struct ast_node *arg = ast_node_create(arg_tok);
				ast_add_arg(node, arg);
				arg_tok = lexer_next_token(lexer, &err);
				if (err != NULL) {
					error_print(err);
					return NULL;
				}
			}

			if (arg_tok.type == TOKEN_COMMAND) {
				token = arg_tok;
				continue;
			}
		}

		token = lexer_next_token(lexer, &err);
		if (err != NULL) {
			error_print(err);
			return NULL;
		}
	}

	struct ast_node *node = ast_node_create(
		(struct token){ .type = TOKEN_EOF, .value = NULL });
	ast_add_arg(root, node);

	return root;
}
