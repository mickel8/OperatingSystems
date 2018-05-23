all: compile

compile:
	gcc -Wall main.c -o main -lpthread -lm
	
run:
	echo "1 THREAD" >> Times.txt
	./main 1 mountain.ascii.pgm filtr.txt out.pgm >> Times.txt
	rm out.pgm
	echo "2 THREAD" >> Times.txt
	./main 2 mountain.ascii.pgm filtr.txt out.pgm >> Times.txt
	rm out.pgm
	echo "4 THREAD" >> Times.txt
	./main 4 mountain.ascii.pgm filtr.txt out.pgm >> Times.txt
	rm out.pgm
	echo "8 THREAD" >> Times.txt
	./main 8 mountain.ascii.pgm filtr.txt out.pgm >> Times.txt
	
rm:
	rm main out.pgm Times.txt
