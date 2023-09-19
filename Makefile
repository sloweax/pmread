CC=cc
CFLAGS=-Wall -Wextra -pedantic -g
NAME=pmread
LIBS=

all: $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) $< -c

$(NAME): $(NAME).o util.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@ 

.PHONY: clean

clean:
	rm -f *.o $(NAME)
