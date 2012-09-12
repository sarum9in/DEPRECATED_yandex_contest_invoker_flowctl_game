#pragma once

#include "yandex/contest/invoker/flowctl/game/Broker.hpp"

#include "yandex/contest/StreamEnum.hpp"

#include <vector>
#include <string>

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/lexical_cast.hpp>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    class BrokerInterface: public Broker
    {
    public:
        virtual void begin(const SolutionId id, const std::string &tokenizerArgument,
                           const ResourceLimits &resourceLimits) override;

        virtual void begin(const SolutionId id, const std::string &tokenizerArgument) override;

        virtual void send(const SolutionId id, const std::string &msg) override;

        virtual Result end(const SolutionId id, const bool discardRemaining) override;

        Result end(const SolutionId id);

        virtual void terminate(const SolutionId id) override;

        void runOnce(Broker &broker);

    protected:
        struct Command
        {
            YANDEX_CONTEST_INCLASS_STREAM_ENUM(Type,
            (
                BEGIN,
                SEND,
                END,
                TERMINATE
            ))

            template <typename Archive>
            void serialize(Archive &ar, const unsigned int)
            {
                ar & BOOST_SERIALIZATION_NVP(type);
                ar & BOOST_SERIALIZATION_NVP(id);
                ar & BOOST_SERIALIZATION_NVP(args);
            }

            Command()=default;

            template <typename ... Args>
            Command(const Type type_, const SolutionId id_, Args &&...args):
                type(type_), id(id_), args{boost::lexical_cast<std::string>(args)...} {}

            Type type = TERMINATE;
            SolutionId id = std::numeric_limits<SolutionId>::max();
            std::vector<std::string> args;
        };

    protected:
        virtual void send(const Command &command)=0;
        virtual void send(const Result &result)=0;

        virtual Command recvCommand()=0;
        virtual Result recvResult()=0;

    private:
        void run(Broker &broker, const Command &command);
    };
}}}}}
