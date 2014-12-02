all: server client

client: client.c libmfs.so udp.o
	gcc -lmfs -L . -o client client.c udp.o -Wall -Werror

server: server.c libmfs.so udp.o
	gcc -lmfs -L . -o server server.c udp.o -Wall -Werror

libmfs.so: mfs.o udp.o
	gcc -shared -o libmfs.so mfs.o udp.o -Wall -Werror
	
mfs.o: mfs.c
	gcc -c -fpic mfs.c -o mfs.o -Wall -Werror
	
udp.o: udp.c
	gcc -c udp.c -o udp.o -Wall -Werror
	
clean:
	rm -rf *.so *.o server