CC=gcc

static: deamon.c libfile_log.a file_log.h
	$(CC) -o logger deamon.c -I. -L. -lfile_log -lsystemd


file_log.o: file_log.c
	$(CC) -c file_log.c -o file_log.o -I. -L. -lsystemd

libfile_log.a: file_log.o
	ar -rv libfile_log.a file_log.o


# Clean all created files
clean:
	rm -r *.o *.a
