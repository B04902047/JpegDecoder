all: main.c
	gcc main.c -o main -lm
run: main
	./main
clean:
	rm main
