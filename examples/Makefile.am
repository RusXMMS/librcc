bin_PROGRAMS= example
example_SOURCES= example.c
example_LDADD= ../src/librcc.la

EXTRA_DIST= input-russian.txt rcc.xml rcc-example.xml

test-russian: example
	cat input-russian.txt | ./example

INCLUDES = -I../src -I../ui
