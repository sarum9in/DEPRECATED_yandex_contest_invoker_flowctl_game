#pragma once

#include <string>
#include <vector>

#include "bunsan/stream_enum.hpp"

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
        BUNSAN_INCLASS_STREAM_ENUM_CLASS(Status,
        (
            FAIL,       ///< An error has occurred.
            CONTINUE,   ///< There is no enough symbols, need more.
            ACCEPTED    ///< Token was completed. \see Result::accepted.
        ))

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

        Result operator()(const std::string &string);

        Result operator()(const std::vector<char> &string);

        template <typename T>
        Result operator()(const T &iterable)
        {
            return (*this)(begin(iterable), end(iterable));
        }

        template <typename Iter>
        Result operator()(const Iter &begin, const Iter &end)
        {
            return (*this)(std::vector<char>(begin, end));
        }

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
