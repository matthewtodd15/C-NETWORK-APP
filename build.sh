#!/bin/bash

OUTPUT="server"
TEST_OUTPUT="test"

gcc -Wall ./src/main.c ./src/http.c -o ${OUTPUT} -fopenmp -lpthread -lcrypto
gcc -Wall ./src/test.c ./src/http.c -o ${TEST_OUTPUT} -fopenmp -lpthread -lcrypto

echo "Built to file: ${OUTPUT}"
echo "Built tests to file: ${TEST_OUTPUT}"
