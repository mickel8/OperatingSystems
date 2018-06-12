all: compile

compile:
	gcc -Wall main.c -o main -lpthread
	gcc -Wall client.c -o client
run:
	./main 8080 myPath
rm:
	rm main client \
