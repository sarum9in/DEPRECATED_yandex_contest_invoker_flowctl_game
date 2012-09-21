#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define TOKENIZER_FAIL -1
#define TOKENIZER_CONTINUE 0
#define TOKENIZER_ACCEPTED 1

/*!
 * \file
 *
 * This algorithm does not work correct on overlapping strings.
 * May be reimplemented using KMP.
 */

#warning "See comments above about implementation."

typedef struct
{
    char *sep;
    size_t sep_size;
    size_t over;
} tokenizer;

void *tokenizer_new(const char *line, size_t size)
{
    assert(line);
    assert(size);
    tokenizer *tok = calloc(1, sizeof(tokenizer));
    if (!tok)
        goto tok_out;
    tok->sep_size = strlen(line);
    tok->sep = malloc(tok->sep_size + 1);
    if (!tok->sep)
        goto sep_out;
    memcpy(tok->sep, line, tok->sep_size + 1);
    tok->over = 0;
    return tok;
    // never get here
    //free(tok->sep);
sep_out:
    free(tok);
tok_out:
    return NULL;
}

void tokenizer_delete(void *tok_)
{
    assert(tok_);
    tokenizer *tok = tok_;
    assert(tok->sep);
    free(tok->sep);
    free(tok);
}

int tokenizer_parse(void *tok_, const char *data, size_t *size)
{
    assert(tok_);
    assert(data);
    assert(size);
    tokenizer *tok = tok_;
    assert(tok->sep);
    assert(tok->sep_size);
    size_t data_size = *size;
    for (size_t i = 0; i < data_size; ++i)
    {
        assert(tok->over <  tok->sep_size);
        if (data[i] == tok->sep[tok->over])
            ++tok->over;
        else
            tok->over = 0;
        if (tok->over == tok->sep_size)
        {
            *size = i + 1;
            return 1;
        }
    }
    return TOKENIZER_CONTINUE;
}
