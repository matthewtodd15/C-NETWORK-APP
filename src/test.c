#include "http.h"
#include<stdio.h>

connection connections[1];
int num_conns = 0;

int main() {
  char data[] = "Hello World";
  char encoded[1024];
  base64_encode(data, sizeof(data) - 1, encoded);
  printf("%s\n", encoded);
  return 0;
}
