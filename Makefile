CC = gcc
CFLAGS = -Wall -Wextra -pthread

all: receptionist visitor monitor initializer closing

receptionist: receptionist.o
	$(CC) $(CFLAGS) -o receptionist receptionist.o

visitor: visitor.o
	$(CC) $(CFLAGS) -o visitor visitor.o

monitor: monitor.o
	$(CC) $(CFLAGS) -o monitor monitor.o

closing: closing.o
	$(CC) $(CFLAGS) -o closing closing.o

initializer: initializer.o
	$(CC) $(CFLAGS) -o initializer initializer.o

visitor.o: visitor.c shared_mem.h logger.h
	$(CC) $(CFLAGS) -c visitor.c

receptionist.o: receptionist.c shared_mem.h logger.h
	$(CC) $(CFLAGS) -c receptionist.c

monitor.o: monitor.c shared_mem.h logger.h
	$(CC) $(CFLAGS) -c monitor.c

closing.o: closing.c shared_mem.h logger.h
	$(CC) $(CFLAGS) -c closing.c

initializer.o: initializer.c shared_mem.h logger.h
	$(CC) $(CFLAGS) -c initializer.c

clean:
	rm -f receptionist visitor monitor initializer closing *.o

#./initializer <number_visitors> <order_time> <rest_time> <KEY>
run:
	./initializer 212 0 0 12347

.PHONY: all clean