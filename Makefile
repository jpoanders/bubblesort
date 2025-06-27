FLAGS=-Wall -Wno-unused-result -std=c11 -O2
CC=gcc
RM=rm -f
EXEC=sort

all: $(EXEC)

sort.o: sort.c
	$(CC) $(FLAGS) -c sort.c -lpthread

thread_pool.o: thread_pool.c
	$(CC) $(FLAGS) -c thread_pool.c -lpthread

$(EXEC): sort.o thread_pool.o
	$(CC) $(FLAGS) sort.o thread_pool.o -o $(EXEC) -lpthread
	$(RM) sort.o thread_pool.o

run:
	./$(EXEC)

clean:
	$(RM) sort.o thread_pool.o $(EXEC)
