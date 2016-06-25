#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tokenizer.h"

/**
 * Get file size from file pointer.
 *
 * \param file
 * file pointer
 *
 * \return
 * file size on success, or -1 on error.
 */
static long get_file_size(FILE *file)
{
    if (fseek(file, 0, SEEK_END) != -1) {
        long size;

        size = ftell(file);
        fseek(file, 0, SEEK_SET);

        return size;
    }

    return -1;
}

/**
 * Read a file into memory.
 *
 * @param filename
 * The filename to read
 *
 * @return
 * The file in memory or NULL on error.
 * The caller must free the memory.
 */
static char *load_file(const char *filename)
{
    FILE *file;
    long file_size;
    char *result = 0;

    file = fopen(filename, "rb");
    if (file)  {
        file_size = get_file_size(file);
        if (file_size > 0) {
            size_t bytes_read;
            result = (char*)malloc(sizeof(char) * (file_size + 1));

            /* Read in the entire file */
            bytes_read = fread(result, 1, file_size, file);

            /* If we had a partial read, bail */
            if (bytes_read != file_size) {
                free(result);
                result = 0;
            } else {
                /* Zero terminate buffer */
                result[bytes_read] = 0;
            }
        }

        fclose(file);
    }

    return result;
}

static void usage(void)
{

    printf("tokenizer_driver <file> <c|d|go|rust|ada>\n");
    exit(-1);
}

int main(int argc, char **argv)
{
    struct tokenizer *t = tokenizer_init();
    int ret;
    enum tokenizer_language_support l = TOKENIZER_LANGUAGE_UNKNOWN;
    struct token_data tok_data;

    if (argc != 3)
        usage();

    if (strcmp(argv[2], "c") == 0)
        l = TOKENIZER_LANGUAGE_C;
    else if (strcmp(argv[2], "asm") == 0)
        l = TOKENIZER_LANGUAGE_ASM;
    else if (strcmp(argv[2], "d") == 0)
        l = TOKENIZER_LANGUAGE_D;
    else if (strcmp(argv[2], "go") == 0)
        l = TOKENIZER_LANGUAGE_GO;
    else if (strcmp(argv[2], "rust") == 0)
        l = TOKENIZER_LANGUAGE_GO;
    else if (strcmp(argv[2], "ada") == 0)
        l = TOKENIZER_LANGUAGE_ADA;
    else
        usage();

    char *buffer = load_file(argv[1]);

    if (tokenizer_set_buffer(t, buffer, l) == -1) {
        printf("%s:%d tokenizer_set_file error\n", __FILE__, __LINE__);
        return -1;
    }

    while ((ret = tokenizer_get_token(t, &tok_data)) > 0) {

        printf("Token:\n");
        printf("\tNumber: %d\n", tok_data.e);
        printf("\tType: %s\n", tokenizer_get_printable_enum(tok_data.e));
        printf("\tData: %s\n", tok_data.data);
    }

    if (ret == 0)
        printf("finished!\n");
    else if (ret == -1)
        printf("Error!\n");

    return 0;
}
