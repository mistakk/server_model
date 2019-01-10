cc = gcc
src = $(wildcard *.c)
target = $(patsubst %.c, %, ${src})
LDFLAGS = -lpthread

.PHONY: all clean

%.o:%.c
	$((cc)) -c -o $@ $(LDFLAGS)
%:%.o
	$((cc)) -o $@
all: ${target}

clean:
	rm -f ${target}

