CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99
LIBS = -lncurses -lpthread -lssl -lcrypto

# Исходные файлы
SRC = src/main.c src/game.c src/network.c src/display.c

# Объектные файлы
OBJ = $(SRC:.c=.o)

TARGET = pong_game_p2p

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean