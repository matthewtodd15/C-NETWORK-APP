#!/bin/bash

OUTPUT="server"

gcc -g -Wall ./src/main.c ./src/http.c -o ${OUTPUT} -lb64 -lpthread -lcrypto

echo "Built to file: ${OUTPUT}"
