#ifndef BASE64_H
#define BASE64_H

typedef enum {
    step_A, step_B, step_C
} base64_step;

typedef struct {
    base64_step step;
    char result;
    int stepcount;
} base64_encodestate;

void base64_init_encodestate(base64_encodestate* state_in);
int base64_encode_block(const char* plaintext_in, int length_in, char* code_out, base64_encodestate* state_in);
int base64_encode_blockend(char* code_out, base64_encodestate* state_in);

#endif