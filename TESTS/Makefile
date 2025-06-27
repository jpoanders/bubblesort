FLAGS=-Wall -Wno-unused-result -std=c11 -O2
CC=gcc
RM=rm -f
EXEC=tester

all: $(EXEC)

tester.o: tester.c
	$(CC) $(FLAGS) -c tester.c -lpthread

sort.o: sort.c
	$(CC) $(FLAGS) -c sort.c -lpthread

merge_sort.o: merge_sort.c
	$(CC) $(FLAGS) -c merge_sort.c -lpthread

$(EXEC): tester.o sort.o merge_sort.o
	$(CC) $(FLAGS) tester.o sort.o merge_sort.o -o $(EXEC) -lpthread
	$(RM) tester.o sort.o merge_sort.o

run:
	./$(EXEC)

clean:
	$(RM) tester.o sort.o merge_sort.o $(EXEC)
