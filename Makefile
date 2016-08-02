CC= gcc
LIBS = -lpsapi
OBJ = resonance.o asmInterpreter.o misc.o
CFLAGS = -std=c11 -mno-ms-bitfields -g
all: resonance

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

resonance: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -rf *.o