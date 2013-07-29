CC = clang
CFLAGS = -std=gnu11 -O3 -I.

GAS = as
GASFLAGS =

LIBS=-lm

DEPS =
OBJ = memcpy_bench.o memcpy_movsb.o memcpy_sse2.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.S $(DEPS)
	$(GAS) $(GASFLAGS) -o $@ $<

memcpy_bench: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o
