all:
	gcc main.c parser/parser.c net/net.c -ljson-c -lresolv -lldns -o dns.out
	./dns.out

clean:
	rm -rf *.out
