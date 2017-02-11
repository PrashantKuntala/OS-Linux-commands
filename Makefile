CC = gcc

all: 
	$(CC) ls_il.c -o ls_il
	$(CC) mycat.c -o mycat

clean:
	rm ls_il
	rm mycat
