CC = cc
CFLAGS = -Wall -Wextra -Werror
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

game: game.c
	$(CC) $(CFLAGS) -o game game.c $(LDFLAGS)

.PHONY: clean
clean:
	rm -f game