all:
	gcc main.c parser/parser.c -ljson-c -o dns.out
	./dns.out