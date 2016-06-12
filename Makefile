CC=gcc
CFLAGS=-g -O0 -Wall -Wextra -I.
#DEPS = linked_list.h
OBJ = ex3_lb.o
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

ex3_lb: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean:
	rm -rf *.o
