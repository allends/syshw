CFLAGS = -g -Wall -Wvla -fsanitize=address -lm

memperf : memperf.c mymalloc.o
	gcc $(FLAGS) -o memperf memperf.c -lm

driver : driver.c	mymalloc.o
	gcc $(CFLAGS) -o driver driver.c -lm

%.o: %.c
	gcc -c -o $@ $<

