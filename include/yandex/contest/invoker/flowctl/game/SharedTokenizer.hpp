#pragma once

#include "yandex/contest/invoker/flowctl/game/Tokenizer.hpp"

#include "yandex/contest/system/unistd/DynamicLibrary.hpp"

#include <cstdint>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    /// \note Stores data related to DynamicLibrary.
    class SharedTokenizerFactory
    {
    public:

        /// Returns not-null pointer. nullptr means out-of-memory.
        typedef void *(*newTokenizerFunction)(const char *);
        typedef void (*deleteTokenizerFunction)(void *);
        typedef int (*parseFunction)(void *, const char *, size_t *);
        typedef const char *(*whatFunction)(void *);

        static constexpr const char *newTokenizerName = "tokenizer_new_tokenizer";
        static constexpr const char *deleteTokenizerName = "tokenizer_delete_tokenizer";
        static constexpr const char *parseName = "tokenizer_parse";
        static constexpr const char *whatName = "tokenizer_what";

    public:
        explicit SharedTokenizerFactory(system::unistd::DynamicLibrary &dl);

        std::unique_ptr<Tokenizer> instance(const std::string &argument);

    private:
        class Instance;

    private:
        newTokenizerFunction newTokenizer_ = nullptr;
        deleteTokenizerFunction deleteTokenizer_ = nullptr;
        parseFunction parse_ = nullptr;
        whatFunction what_ = nullptr;
    };
}}}}}
