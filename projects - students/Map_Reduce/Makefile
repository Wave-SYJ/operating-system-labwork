build : wordcount.o arraylist.o mapreduce.o 
	cc -o wordcount wordcount.o arraylist.o mapreduce.o -Wall -Werror -pthread -O
	rm *.o

arraylist.o : arraylist.c arraylist.h
	cc -c arraylist.c

mapreduce.o : mapreduce.c mapreduce.h arraylist.h
	cc -c mapreduce.c

wordcount.o : wordcount.c mapreduce.h
	cc -c wordcount.c

clean :
	rm wordcount *.o

.PHONY : build clean