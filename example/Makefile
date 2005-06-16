all: example

example: example.o
	gcc -o example -lrcd example.o

test: example
	cat input.txt | ./example

clean:
	rm -f example example.o