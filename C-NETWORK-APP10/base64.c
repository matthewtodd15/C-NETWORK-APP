#include "base64.h"
#include <string.h>

void base64_init_encodestate(base64_encodestate* state_in) {
    state_in->step = step_A;
    state_in->result = 0;
    state_in->stepcount = 0;
}

int base64_encode_block(const char* plaintext_in, int length_in, char* code_out, base64_encodestate* state_in) {
    static const char* encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const unsigned char* plain = (const unsigned char*)plaintext_in;
    int count = 0;
    unsigned char result;

    while (length_in-- > 0) {
        switch (state_in->step) {
            case step_A:
                result = (plain[0] >> 2) & 0x3F;
                code_out[count++] = encoding[result];
                state_in->result = (plain[0] & 0x03) << 4;
                state_in->step = step_B;
                break;
            case step_B:
                result = state_in->result | ((plain[0] >> 4) & 0x0F);
                code_out[count++] = encoding[result];
                state_in->result = (plain[0] & 0x0F) << 2;
                state_in->step = step_C;
                break;
            case step_C:
                result = state_in->result | ((plain[0] >> 6) & 0x03);
                code_out[count++] = encoding[result];
                result = plain[0] & 0x3F;
                code_out[count++] = encoding[result];
                state_in->step = step_A;
                break;
        }
        plain++;
    }

    return count;
}

int base64_encode_blockend(char* code_out, base64_encodestate* state_in) {
    int count = 0;
    switch (state_in->step) {
        case step_B:
            code_out[count++] = '=';
            code_out[count++] = '=';
            break;
        case step_C:
            code_out[count++] = '=';
            break;
        case step_A:
            break;
    }
    return count;
}