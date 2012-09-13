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

        /*!
         * \brief Create new Tokenizer.
         *
         * \param argument
         * \param size size of argument
         *
         * \return Non-null pointer (nullptr means out-of-memory).
         */
        typedef void *(*newTokenizerFunction)(const char *, size_t);

        /*!
         * \brief Free memory allocated by newTokenizerFunction.
         *
         * \param tokenizer non-null pointer returned by newTokenizerFunction.
         */
        typedef void (*deleteTokenizerFunction)(void *);

        /*!
         * \brief Parse another part of solution's output.
         *
         * \param tokenizer
         * \param data string, written by solution
         * \param size size of string, written by solution
         *
         * \return -1 on fail, 0 on continue, 1 on accept
         * (in this case, size should be changed to number of bytes accepted).
         */
        typedef int (*parseFunction)(void *, const char *, size_t *);

        /*!
         * \brief Show human-readable error message.
         *
         * \return nullptr or null-terminated string with error message
         * (should be valid until tokenizer is deleted).
         *
         * \note May be undefined.
         * In that case default implementation of Tokenizer::what() will be used.
         */
        typedef const char *(*whatFunction)(void *);

        static constexpr const char *newTokenizerName = "tokenizer_new";
        static constexpr const char *deleteTokenizerName = "tokenizer_delete";
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
