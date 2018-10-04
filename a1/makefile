FLAGS = -Iinclude
all: client server

client: src/client.o
	gcc src/client.o $(FLAGS) -o client

server: src/server.o src/networkHelpers.o
	gcc src/server.o src/networkHelpers.o $(FLAGS) -o server

clean:
	rm client server src/*.o

run20x:
	./run.sh localhost:8080 data/smallTest.txt 20

%.o : %.c
	gcc -c $*.c $(FLAGS) -o $*.o
