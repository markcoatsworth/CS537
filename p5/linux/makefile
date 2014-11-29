all: server client

client: client.c libmfs.so
	gcc -lmfs -L . -o client client.c -Wall -Werror

server: server.c libmfs.so
	gcc -lmfs -L . -o server server.c -Wall -Werror

libmfs.so: mfs.o
	gcc -shared -o libmfs.so mfs.o -Wall -Werror
	
mfs.o: mfs.c
	gcc -c -fpic mfs.c -o mfs.o -Wall -Werror
	
clean:
	rm -rf *.so *.o server