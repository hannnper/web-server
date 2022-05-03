CC=gcc
CFLAGS=-Wall -g -O3
LDFLAGS=-lpthread
EXE_SERVER=server
OBJ_SERVER=utils.o

all: $(EXE_SERVER)

$(EXE_SERVER): server.c utils.h $(OBJ_SERVER)
	$(CC) $(CFLAGS) -o $(EXE_SERVER) $< $(OBJ_SERVER) $(LDFLAGS)

%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o $(EXE_SERVER)

format:
	clang-format -i *.c *.h
