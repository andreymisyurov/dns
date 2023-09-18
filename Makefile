all: build run

build:
	gcc main.c parser/parser.c net/net.c -ljson-c -lresolv -o dns.out

run:
	./dns.out

clean:
	rm -rf *.out

docker_build:
	docker build -t dns-server .

docker_run:
	docker network create dns
	docker run --rm --network dns -d -it -p 5354:53/udp dns-server
