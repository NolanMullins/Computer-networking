
all: client server

client: src/client.c
	gcc src/client.c -o client -Iinclude

server: src/server.c
	gcc src/server.c -o server -std=c99 -Iinclude

clean:
	rm client server

run20x:
	./run.sh localhost:8080 data/smallTest.txt 20


