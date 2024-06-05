#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "parser.h"
#include "interpreter.h"
#include "command.h"
#include "command_funcs.h"
#include <assert.h>

char *read_file_to_str(const char *filename)
{
	FILE *file = fopen(filename, "r");
	assert(file != NULL);

	fseek(file, 0, SEEK_END);
	long len = ftell(file);
	fseek(file, 0, SEEK_SET);

	char *buf = malloc(len + 1);
	assert(buf != NULL);

	fread(buf, 1, len, file);
	buf[len] = '\0';

	fclose(file);

	return buf;
}

void ext_fail()
{
	fprintf(stderr, "%s\n",
		"ERROR Interpreting: Failed to interpret the code exit code: 1");
	exit(EXT_FAIL);
}

int main(int argc, const char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		ext_fail();
	}

	const char *filename = argv[1];
	char *file_content = read_file_to_str(filename);

	struct command_base *cb = command_base_create();
	command_register(cb, "PRINT", command_func_print);
	command_register(cb, "CREATE", command_func_create);
	command_register(cb, "SET", command_func_set);

	subcommand_register(cb, "ADD", subcommand_func_add);

	struct lexer *lexer = lexer_create(file_content, cb);

	struct ast_node *ast = parse_tokens(lexer);
	if (ast == NULL) {
		ext_fail();
	}

	if (interpret_ast(ast, cb) == EXT_FAIL) {
		lexer_destroy(lexer);
		ast_free(ast);
		free(file_content);

		ext_fail();
	}

	lexer_destroy(lexer);
	ast_free(ast);
	free(file_content);
	return EXT_SUCCESS;
}
