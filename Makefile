CC = gcc
CFLAGS = -Wall -lpthread 

all: pandaria


threadpool.o: threadpool.c 
	$(CC) -c $^ $(CFLAGS)
	
common.o: common.c
	$(CC) -c $^

pandaria: common.o threadpool.o pandaria.c
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm *.o pandaria *~ -rf
