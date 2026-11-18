CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g

test: test_circular_queue
	./test_circular_queue

test_circular_queue: circular_queue.c test_circular_queue.c circular_queue.h
	$(CC) $(CFLAGS) circular_queue.c test_circular_queue.c -o $@ -lm

sanitize: circular_queue.c test_circular_queue.c
	$(CC) $(CFLAGS) -fsanitize=address,undefined circular_queue.c test_circular_queue.c -o test_asan -lm && ./test_asan

clean:
	rm -f test_circular_queue test_asan
