#ifndef PARSER_H
#define PARSER_H

struct ast_node;
struct lexer;

struct ast_node *parse_tokens(struct lexer *lexer);

#endif
