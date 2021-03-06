#pragma once

#include <yandex/contest/invoker/flowctl/game/Broker.hpp>
#include <yandex/contest/invoker/flowctl/game/SharedTokenizer.hpp>

#include <yandex/contest/system/unistd/DynamicLoader.hpp>

#include <bunsan/stream_enum.hpp>

#include <boost/noncopyable.hpp>

#include <limits>
#include <memory>
#include <string>
#include <vector>

#include <ext/stdio_filebuf.h>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    class BrokerImpl: public Broker
    {
    public:
        struct Process
        {
            template <typename Archive>
            void serialize(Archive &ar, const unsigned int)
            {
                ar & BOOST_SERIALIZATION_NVP(in);
                ar & BOOST_SERIALIZATION_NVP(out);
                ar & BOOST_SERIALIZATION_NVP(id);
            }

            int in, out;
            ProcessId id;
        };

        struct IOInterface
        {
            explicit IOInterface(const Process &process);

            __gnu_cxx::stdio_filebuf<char> inputBuf, outputBuf;
            std::istream input;
            std::ostream output;
        };

        struct Options
        {
            BUNSAN_INCLASS_STREAM_ENUM_CLASS(Protocol,
            (
                TEXT
            ))

            template <typename Archive>
            void serialize(Archive &ar, const unsigned int)
            {
                ar & BOOST_SERIALIZATION_NVP(protocol);
                ar & BOOST_SERIALIZATION_NVP(tokenizer);
                ar & BOOST_SERIALIZATION_NVP(solutions);
                ar & BOOST_SERIALIZATION_NVP(killer);
                ar & BOOST_SERIALIZATION_NVP(judge);
                ar & BOOST_SERIALIZATION_NVP(defaultResourceLimits);
            }

            Protocol protocol;
            boost::filesystem::path tokenizer;
            std::vector<Process> solutions;
            Process killer, judge;
            ResourceLimits defaultResourceLimits;
            TokenizerArgument defaultTokenizerArgument;
        };

    public:
        explicit BrokerImpl(const Options &options);

        virtual void begin(const SolutionId id,
                           const TokenizerArgument &tokenizerArgument,
                           const ResourceLimits &resourceLimits) override;

        virtual void begin(const SolutionId id,
                           const TokenizerArgument &tokenizerArgument) override;

        virtual void begin(const SolutionId id) override;

        virtual void send(const SolutionId id, const std::string &msg) override;

        virtual Result end(const SolutionId id, const bool discardRemaining) override;

        virtual void terminate(const SolutionId id) override;

    private:
        struct Solution
        {
            explicit Solution(const Process &process_):
                process(process_) {}

            Process process;
            std::unique_ptr<Tokenizer> tokenizer;
            Tokenizer::Status tokenizerStatus;
            std::string result;
            std::deque<char> inbuf, outbuf;
            ResourceLimits resourceLimits;
        };

    private:
        Solution &solution(const SolutionId id);

        /// \return resourceLimits-related statuses or OK.
        Result::Status communicate(const SolutionId id);

        Killer::Status freeze(const SolutionId id);
        Killer::Status unfreeze(const SolutionId id);

    private:
        const ResourceLimits defaultResourceLimits_;
        const TokenizerArgument defaultTokenizerArgument;
        system::unistd::DynamicLibrary tokenizerLibrary_;
        SharedTokenizerFactory sharedTokenizerFactory_;
        IOInterface killerIO_;
        std::unique_ptr<KillerInterface> killer_;
        std::vector<Solution> solutions_;
    };
}}}}}
