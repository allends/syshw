CFLAGS = -g -Wall -Wvla -fsanitize=address -lm


driver : driver.c	mymalloc.o
	gcc $(CFLAGS) -o driver driver.c -lm

%.o: %.c
	gcc -c -o $@ $<

