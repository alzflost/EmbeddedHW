
CC=arm-none-linux-gnueabi-gcc -static
CFLAGS=-Wall -Wextra -pedantic -std=c11

EXECUTABLE=hw1execute

OBJECTS=main.o io.o merge.o

HDR = hw1.h

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXECUTABLE)

main.o: main.c $(HDR)
	$(CC) $(CFLAGS) -c main.c

io.o: io.c $(HDR)
	$(CC) $(CFLAGS) -c io.c

merge.o: merge.c $(HDR)
	$(CC) $(CFLAGS) -c merge.c

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
