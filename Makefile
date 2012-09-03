CC = gcc
OBJS = stopwatch.o
#CFLAGS = -O3
CFLAGS = -g
LDFLAGS = 

stopwatch: $(OBJS)
	gcc -o stopwatch $(LDFLAGS) $< -lm -lrt

%.o: %.c
	gcc -c $(CFLAGS) -o $@ $<

clean:
	rm -f $(OBJS) stopwatch
