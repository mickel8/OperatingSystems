all: compile

compile:
	gcc -Wall golibroda.c -o golibroda -lrt -pthread
	gcc -Wall supervisor.c -o supervisor -lrt -pthread
	gcc -Wall client.c -o client -lrt -pthread

rm:
	rm golibroda supervisor client
