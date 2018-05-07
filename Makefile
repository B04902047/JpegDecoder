all: main.c
	gcc main.c -o main.out
run: main
	./main.out
clean:
	rm main.out
