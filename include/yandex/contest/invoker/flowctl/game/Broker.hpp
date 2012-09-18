#pragma once

#include "yandex/contest/invoker/flowctl/game/Killer.hpp"
#include "yandex/contest/invoker/flowctl/game/KillerInterface.hpp"
#include "yandex/contest/invoker/flowctl/game/Tokenizer.hpp"

#include "yandex/contest/StreamEnum.hpp"

#include <vector>
#include <string>
#include <limits>
#include <memory>

#include <boost/noncopyable.hpp>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    class Broker: private boost::noncopyable
    {
    public:
        typedef std::size_t SolutionId;
        typedef Killer::Id ProcessId;
        typedef std::string TokenizerArgument;

        struct ResourceLimits
        {
            template <typename Archive>
            void serialize(Archive &ar, const unsigned int)
            {
                ar & BOOST_SERIALIZATION_NVP(timeLimitMillis);
                ar & BOOST_SERIALIZATION_NVP(realTimeLimitMillis);
            }

            ResourceLimits()=default;

            ResourceLimits(const std::uint64_t timeLimitMillis_,
                           const std::uint64_t realTimeLimitMillis_):
                timeLimitMillis(timeLimitMillis_),
                realTimeLimitMillis(realTimeLimitMillis_) {}

            std::uint64_t timeLimitMillis = std::numeric_limits<std::uint64_t>::max();
            std::uint64_t realTimeLimitMillis = std::numeric_limits<std::uint64_t>::max();
        };

        struct Result
        {
            YANDEX_CONTEST_INCLASS_STREAM_ENUM_CLASS(Status,
            (
                OK,
                TOKENIZER_ERROR,
                EOF_ERROR,
                TIME_LIMIT_EXCEEDED,
                REAL_TIME_LIMIT_EXCEEDED
            ))

            template <typename Archive>
            void serialize(Archive &ar, const unsigned int)
            {
                ar & BOOST_SERIALIZATION_NVP(status);
                ar & BOOST_SERIALIZATION_NVP(data);
            }

            Status status;

            /*!
             * \brief Depending on status.
             *
             * If OK -> returned data, otherwise error message.
             */
            std::string data;
        };

    public:
        /*!
         * \brief Begin iteration.
         *
         * Create Tokenizer, input buffer.
         *
         * \warning The moment of solution thaw is not specified.
         * Moreover, in some cases it may stay frozen (e.g. using
         * data from previous iteration).
         *
         * \warning Current implementation does not support
         * more then one active solution (solution is active after begin()
         * and inactive after end(), solution is inactive by default).
         */
        virtual void begin(const SolutionId id, const TokenizerArgument &tokenizerArgument,
                           const ResourceLimits &resourceLimits)=0;

        /// begin(id, tokenizerArgument, defaultResourceLimits)
        virtual void begin(const SolutionId id, const TokenizerArgument &tokenizerArgument)=0;

        /// begin(id, defaultTokenizerArgument)
        virtual void begin(const SolutionId id)=0;

        /*!
         * \brief Send data.
         */
        virtual void send(const SolutionId id, const std::string &msg)=0;

        /*!
         * \brief End iteration.
         *
         * \param discardRemaining Discard all data not read by solution.
         *
         * Save all remaining data for the next iteration
         * written by solution but not read by Tokenizer.
         */
        virtual Result end(const SolutionId id, const bool discardRemaining=false)=0;

        /*!
         * \brief Terminate specified process.
         *
         * Close all related resources,
         * ends iteration (if proceeding).
         */
        virtual void terminate(const SolutionId id)=0;

        virtual ~Broker();
    };
}}}}}
