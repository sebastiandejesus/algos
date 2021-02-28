CC = gcc
RM = rm -f

CFLAGS = -ggdb3 -Wall -Werror -fvisibility=hidden
CPPFLAGS = -I include

sources = $(shell find ./src -name '*.c')
objects = $(subst .c,.o,$(sources)) 

all: $(objects)

$(objects): $(sources)

clean:
	$(RM) $(objects)

.PHONY: clean
