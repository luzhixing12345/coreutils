
#include <stdio.h>
#include <string.h>

#include "xbox/xargparse.h"
#include "xbox/xbox.h"
#include "xbox/xutils.h"

int decode = 0;

static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int base64_index(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A';
    } else if (c >= 'a' && c <= 'z') {
        return c - 'a' + 26;
    } else if (c >= '0' && c <= '9') {
        return c - '0' + 52;
    } else if (c == '+') {
        return 62;
    } else if (c == '/') {
        return 63;
    } else {
        return -1;
    }
}

/**
 * @brief base64 编码
 *
 * @param data
 * @param input_length
 * @param output_length
 * @return char*
 */
char *XBOX_base64_encode(const char *str) {
    size_t output_length;
    size_t input_length = strlen(str);
    output_length = 4 * ((input_length + 2) / 3);

    char *encoded_data = malloc(output_length + 1);
    if (encoded_data == NULL) {
        return NULL;
    }

    for (size_t i = 0, j = 0; i < input_length;) {
        u_int32_t octet_a = i < input_length ? str[i++] : 0;
        u_int32_t octet_b = i < input_length ? str[i++] : 0;
        u_int32_t octet_c = i < input_length ? str[i++] : 0;

        u_int32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = base64_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = base64_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = base64_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = base64_table[(triple >> 0 * 6) & 0x3F];
    }

    // Add padding if necessary
    size_t padding_length =
        output_length - (input_length % 3 == 0 ? output_length : output_length - (3 - input_length % 3));
    for (size_t i = 0; i < padding_length; i++) {
        encoded_data[output_length - padding_length + i] = '=';
    }

    encoded_data[output_length] = '\0';

    return encoded_data;
}

char *XBOX_base64_decode(const char *str) {
    size_t input_length = strlen(str);
    size_t output_length;
    if (input_length % 4 != 0) {
        return NULL;
    }
    output_length = input_length / 4 * 3;
    if (str[input_length - 1] == '=') {
        (output_length)--;
    }
    if (str[input_length - 2] == '=') {
        (output_length)--;
    }

    char *decoded_data = malloc(output_length);
    if (decoded_data == NULL) {
        return NULL;
    }

    for (size_t i = 0, j = 0; i < input_length;) {
        u_int32_t sextet_a = str[i] == '=' ? 0 & i++ : base64_index(str[i++]);
        u_int32_t sextet_b = str[i] == '=' ? 0 & i++ : base64_index(str[i++]);
        u_int32_t sextet_c = str[i] == '=' ? 0 & i++ : base64_index(str[i++]);
        u_int32_t sextet_d = str[i] == '=' ? 0 & i++ : base64_index(str[i++]);

        u_int32_t triple = (sextet_a << 3 * 6) + (sextet_b << 2 * 6) + (sextet_c << 1 * 6) + (sextet_d << 0 * 6);

        if (j < output_length)
            decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < output_length)
            decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < output_length)
            decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return decoded_data;
}

void XBOX_base64(const char *str) {
    if (decode) {
        char *decoded_data = XBOX_base64_decode(str);
        printf("%s\n", decoded_data);
        free(decoded_data);
        return;
    }
    char *encoded_data = XBOX_base64_encode(str);
    printf("%s\n", encoded_data);
    free(encoded_data);
}

int main(int argc, const char **argv) {
    char *input_str = NULL;
    argparse_option options[] = {XBOX_ARG_BOOLEAN(NULL, [-h][--help][help = "show help information"]),
                                 XBOX_ARG_BOOLEAN(NULL, [-v][--version][help = "show version"]),
                                 XBOX_ARG_BOOLEAN(&decode, [-d][--decode][help = "decode data"]),
                                 XBOX_ARG_STR_GROUP(&input_str, [name = "input"]),
                                 XBOX_ARG_END()};
    XBOX_argparse parser;
    XBOX_argparse_init(&parser, options, XBOX_ARGPARSE_ENABLE_ARG_STICK);
    XBOX_argparse_describe(
        &parser, "base64", "Base64 encode or decode FILE, or standard input, to standard output.", "");
    XBOX_argparse_parse(&parser, argc, argv);

    if (XBOX_ismatch(&parser, "help")) {
        XBOX_argparse_info(&parser);
        XBOX_free_argparse(&parser);
        return 0;
    }

    if (XBOX_ismatch(&parser, "version")) {
        printf("%s\n", XBOX_VERSION);
        XBOX_free_argparse(&parser);
        return 0;
    }

    if (input_str) {
        XBOX_base64(input_str);
        free(input_str);
    } else {
        char input_str[XBOX_MAX_INPUT_SIZE];
        fgets(input_str, XBOX_MAX_INPUT_SIZE, stdin);
        XBOX_base64(input_str);
    }
    XBOX_free_argparse(&parser);
    return 0;
}