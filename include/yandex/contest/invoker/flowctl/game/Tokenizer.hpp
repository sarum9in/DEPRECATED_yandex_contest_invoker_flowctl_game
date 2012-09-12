#pragma once

#include <cstdlib>

#include <boost/noncopyable.hpp>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    /*!
     * \warning Have to be reentrant, side-effect-free.
     */
    class Tokenizer: private boost::noncopyable
    {
    public:
        enum class Status
        {
            FAIL = -1,      ///< An error has occurred.
            CONTINUE = 0,   ///< There is no enough symbols, need more.
            ACCEPTED = 1    ///< Token was completed. \see Result::accepted.
        };

        struct Result
        {
            Status status;

            /*!
             * \brief If status == Status::ACCEPTED this field is initialized
             * by number of accepted characters (from current call, i.e. 0 <= accepted <= size).
             */
            std::size_t accepted;
        };

    public:
        virtual ~Tokenizer();

        /*!
         * \warning data is not a null-terminated string.
         */
        virtual Result operator()(const char *data, std::size_t size)=0;

        /*!
         * \brief If error has occurred, Tokenizer may return human-readable explanation.
         *
         * Returned pointer should stay valid between consequent what() calls
         * and until Tokenizer destruction.
         *
         * \return Null-terminated string or nullptr.
         */
        virtual const char *what() const noexcept;
    };
}}}}}
