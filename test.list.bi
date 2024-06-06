:i count 16
:b shell 35
./interpreter tests/hello_world.duc
:i returncode 0
:b stdout 14
Hello, World!

:b stderr 0

:b shell 33
./interpreter tests/int_print.duc
:i returncode 0
:b stdout 5
1896

:b stderr 0

:b shell 31
./interpreter tests/int_var.duc
:i returncode 0
:b stdout 4
500

:b stderr 0

:b shell 27
./interpreter tests/msg.duc
:i returncode 0
:b stdout 36
This is a message by "msg" variable

:b stderr 0

:b shell 32
./interpreter tests/dupe_var.duc
:i returncode 1
:b stdout 0

:b stderr 211
Interpreter Error at line 2, column 8: Found a variable with same identifier 'dupe_identifier', this identifier was first used in line: 1, column: 8
ERROR Interpreting: Failed to interpret the code exit code: 1

:b shell 34
./interpreter tests/int_exceed.duc
:i returncode 1
:b stdout 0

:b stderr 136
Interpreter Error at line 1, column 20: Integer exceeds the max int limit
ERROR Interpreting: Failed to interpret the code exit code: 1

:b shell 42
./interpreter tests/invalid_identifier.duc
:i returncode 1
:b stdout 0

:b stderr 137
Lexer Error at line 2, column 11: Invalid identifier starting with a digit
ERROR Interpreting: Failed to interpret the code exit code: 1

:b shell 27
./interpreter tests/set.duc
:i returncode 0
:b stdout 39
my_var:
500
After setting my_var:
1896

:b stderr 0

:b shell 28
./interpreter tests/swap.duc
:i returncode 0
:b stdout 99
BEFORE SWAP:
msg:
HI
another_msg:
HOO HOO
===============
AFTER SWAP:
msg:
HOO HOO
another_msg:
HI

:b stderr 0

:b shell 34
./interpreter tests/simple_add.duc
:i returncode 0
:b stdout 4
450

:b stderr 0

:b shell 36
./interpreter tests/advanced_add.duc
:i returncode 0
:b stdout 12
sum + 10
25

:b stderr 0

:b shell 39
./interpreter tests/deep_nested_add.duc
:i returncode 0
:b stdout 4
350

:b stderr 0

:b shell 38
./interpreter tests/multiplication.duc
:i returncode 0
:b stdout 5
1924

:b stderr 0

:b shell 35
./interpreter tests/subtraction.duc
:i returncode 0
:b stdout 4
139

:b stderr 0

:b shell 32
./interpreter tests/division.duc
:i returncode 0
:b stdout 3
20

:b stderr 0

:b shell 42
./interpreter tests/complex_arithmetic.duc
:i returncode 0
:b stdout 6
-5015

:b stderr 0

