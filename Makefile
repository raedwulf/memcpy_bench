CC = clang
CFLAGS = -std=gnu11 -I.

GAS = as
GASFLAGS =

LIBS=-lm

DEPS = memcpy_bench.h
OBJ = memcpy_bench.o memcpy_movsb.o memcpy_sse2.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.S $(DEPS)
	$(GAS) $(GASFLAGS) -o $@ $<

memcpy_bench: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o
