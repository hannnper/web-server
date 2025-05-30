CC=gcc
CFLAGS=-Wall -g -O3
LDFLAGS=
EXE_SERVER=server
OBJ_SERVER=utils.o request.o response.o

all: $(EXE_SERVER)

$(EXE_SERVER): server.c utils.h request.h response.h $(OBJ_SERVER)
	$(CC) $(CFLAGS) -o $(EXE_SERVER) $< $(OBJ_SERVER) $(LDFLAGS)

%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o $(EXE_SERVER)

format:
	clang-format -i *.c *.h
