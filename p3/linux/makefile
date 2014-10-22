all: main

main: main.c libmem1.so libmem2.so libmem3.so
	gcc -lmem1 -lmem2 -lmem3 -L . -o main main.c -Wall -Werror
	
libmem1.so: mem.o
	gcc -shared -o libmem1.so mem.o -Wall -Werror

libmem2.so: mem.o
	gcc -shared -o libmem2.so mem.o -Wall -Werror

libmem3.so: mem.o
	gcc -shared -o libmem3.so mem.o -Wall -Werror
	
mem.o: mem.c
	gcc -c -fpic mem.c -o mem.o -Wall -Werror
	
clean:
	rm -rf *.so *.o main