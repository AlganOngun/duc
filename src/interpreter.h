#ifndef INTERPRETER_H
#define INTERPRETER_H

struct ast_node;
struct command_base;

enum EXT_CODE { EXT_SUCCESS, EXT_FAIL };

enum EXT_CODE interpret_ast(const struct ast_node *const ast,
			    struct command_base *cb);

#endif
