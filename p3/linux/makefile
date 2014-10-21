all: main

main: main.c libmem.so
	gcc -lmem -L . -o main main.c -Wall -Werror
	
libmem.so: mem.o
	gcc -shared -o libmem.so mem.o -Wall -Werror
	
mem.o: mem.c
	gcc -c -fpic mem.c -o mem.o -Wall -Werror
	
clean:
	rm -rf *.so *.o main