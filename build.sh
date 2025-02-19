#!/bin/bash

OUTPUT="server"

gcc -g -Wall ./src/*.c -o ${OUTPUT} -lb64 -lcrypto

echo "Built to file: ${OUTPUT}"
