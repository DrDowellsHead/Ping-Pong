#!/bin/bash

echo "Compiling Pong Game"

gcc -o pong_game main.c game.c network_ssl.c display.c -lncurses -lssl -lcrypto -lpthread -std=c11 -Wall

if [ $? -eq 0 ]; then
    echo "Compilation succesful!"
    echo "Run: ./pong_game"
else
    echo "Compilation failed!"  
fi