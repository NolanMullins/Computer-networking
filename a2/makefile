FLAGS = -Iinclude -pthread
CLIENT = src/client.o
SERVER = src/server.o src/networkHelpers.o src/list.o
all: client server

client: $(CLIENT)
	gcc $(CLIENT) $(FLAGS) -o client

server: $(SERVER)
	gcc $(SERVER) $(FLAGS) -o server

clean:
	rm client server src/*.o

cleanOutput:
	rm output/!(out)

run: 
	./client localhost:8080 data/lessSmallTest.txt 128

runBig:
	./client localhost:8080 data/wonderlandBig.txt 128

run20x:
	./run.sh localhost:8080 data/wonderlandBig.txt 20

runSever20x:
	./run.sh 131.104.48.74:12059 data/wonderland.txt 20

%.o : %.c
	gcc -c $*.c $(FLAGS) -o $*.o
