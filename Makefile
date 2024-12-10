# Makefile
CC = gcc
CFLAGS = -Wall -g
LIBS = -lsqlite3 -lcurl -lssl -lcrypto

all: main

main: main.o database.o auth.o api.o cJSON.o
	$(CC) $(CFLAGS) -o main main.o database.o auth.o api.o cJSON.o $(LIBS)

main.o: main.c database.h auth.h api.h
	$(CC) $(CFLAGS) -c main.c

database.o: database.c database.h
	$(CC) $(CFLAGS) -c database.c

auth.o: auth.c auth.h database.h
	$(CC) $(CFLAGS) -c auth.c

api.o: api.c api.h cJSON.h
	$(CC) $(CFLAGS) -c api.c

cJSON.o: cJSON.c cJSON.h
	$(CC) $(CFLAGS) -c cJSON.c

clean:
	rm -f *.o main
