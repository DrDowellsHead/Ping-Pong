CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11
LIBS = -lncurses -lpthread -lssl -lcrypto

# Исходные файлы
SRC = main.c game.c network_ssl.c display.c

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