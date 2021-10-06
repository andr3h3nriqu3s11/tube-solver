.PHONY: run debug clean

run: main
	./main

debug: main
	gdb main

clean:
	rm main

main: main.c
	gcc main.c -o3 main
