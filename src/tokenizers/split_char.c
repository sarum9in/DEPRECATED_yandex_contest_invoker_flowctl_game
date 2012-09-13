#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define TOKENIZER_FAIL -1
#define TOKENIZER_CONTINUE 0
#define TOKENIZER_ACCEPTED 1

typedef struct
{
    char sep;
} tokenizer;

void *tokenizer_new(const char *line, size_t size)
{
    assert(line);
    assert(size == 1);
    tokenizer *tok = malloc(sizeof(tokenizer));
    if (!tok)
        return NULL;
    tok->sep = line[0];
    return tok;
}

void tokenizer_delete(void *tok)
{
    assert(tok);
    free(tok);
}

int tokenizer_parse(void *tok_, const char *data, size_t *size)
{
    assert(tok_);
    assert(data);
    assert(size);
    tokenizer *tok = tok_;
    const char *pos = memchr(data, (unsigned char)tok->sep, *size);
    if (pos)
    {
        // we accept character too
        *size = pos - data + 1;
        return TOKENIZER_ACCEPTED;
    }
    else
    {
        // no separator was found
        return TOKENIZER_CONTINUE;
    }
}
