all: program1 program2 program3

program1: client.c
	gcc client.c -o client -Wall -pthread

program2: server.c
	gcc server.c -o server -Wall -pthread

program3: servant.c
	gcc servant.c -o servant -Wall -pthread

clean:
	rm -f *.o servant server client
