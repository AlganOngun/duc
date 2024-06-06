#include "ast.h"
#include "lexer.h"
#include "error.h"
#include "stdio.h"

static void parse_command(struct lexer *lexer, struct token curr_tok,
			  struct ast_node *node, struct error **err);

struct ast_node *parse_tokens(struct lexer *lexer)
{
	struct error *err = NULL;
	struct token curr_tok = lexer_next_token(lexer, &err);
	if (err != NULL) {
		error_print(err);
		return NULL;
	}

	struct ast_node *root = ast_node_create(
		(struct token){ .type = TOKEN_START, .value = "PROG" });

	while (curr_tok.type != TOKEN_EOF) {
		struct ast_node *curr_node = ast_node_create(curr_tok);
		if (curr_tok.type == TOKEN_COMMAND) {
			parse_command(lexer, curr_tok, curr_node, &err);
			if (err != NULL) {
				error_print(err);
				return NULL;
			}
		}
		ast_add_arg(root, curr_node);

		curr_tok = lexer_next_token(lexer, &err);
		if (err != NULL) {
			error_print(err);
			return NULL;
		}
	}
	struct ast_node *curr_node = ast_node_create(curr_tok);
	ast_add_arg(root, curr_node); // Add EOF

	return root;
}

static void parse_command(struct lexer *lexer, struct token curr_tok,
			  struct ast_node *node, struct error **err)
{
	struct error *internal_err = NULL;
	while (node->argc < curr_tok.max_argc) {
		struct token tok = lexer_next_token(lexer, &internal_err);
		if (internal_err != NULL) {
			break;
		}
		struct ast_node *curr_node = ast_node_create(tok);
		if (tok.type == TOKEN_SUBCOMMAND) {
			ast_add_arg(node, curr_node);

			parse_command(lexer, tok, curr_node, &internal_err);
			if (internal_err != NULL) {
				break;
			}
			continue;
		}

		ast_add_arg(node, curr_node);
	}
	*err = internal_err;
}
