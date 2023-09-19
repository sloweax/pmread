CC=cc
CFLAGS=-Wall -Wextra -pedantic -g
NAME=pmread
LIBS=
BINDSTPATH=/usr/local/bin

all: $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) $< -c

$(NAME): $(NAME).o util.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@ 

.PHONY: clean install uninstall

clean:
	rm -f *.o $(NAME)

install: $(NAME)
	cp $(NAME) $(BINDSTPATH)

uninstall:
	rm -f $(BINDSTPATH)/$(NAME)
