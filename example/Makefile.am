bin_PROGRAMS= example
example_SOURCES= example.c
example_LDADD= @RCD_LIBS@
EXTRA_DIST= input.txt

test: example
	cat input.txt | ./example
