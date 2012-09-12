#include "yandex/contest/invoker/flowctl/game/BrokerInterace.hpp"

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    void BrokerInterface::runOnce(Broker &broker)
    {
        run(broker, recvCommand());
    }

    void BrokerInterface::begin(const SolutionId id, const std::string &tokenizerArgument,
                                const ResourceLimits &resourceLimits)
    {
        send(Command(Command::BEGIN, id, tokenizerArgument,
                     resourceLimits.timeLimitMillis, resourceLimits.realTimeLimitMillis));
    }

    void BrokerInterface::begin(const SolutionId id, const std::string &tokenizerArgument)
    {
        send(Command(Command::BEGIN, id, tokenizerArgument));
    }

    void BrokerInterface::send(const SolutionId id, const std::string &msg)
    {
        send(Command(Command::SEND, id));
    }

    BrokerInterface::Result BrokerInterface::end(const SolutionId id, const bool discardRemaining)
    {
        send(Command(Command::END, id, discardRemaining));
        return recvResult();
    }

    BrokerInterface::Result BrokerInterface::end(const SolutionId id)
    {
        send(Command(Command::END, id));
        return recvResult();
    }

    void BrokerInterface::terminate(const SolutionId id)
    {
        send(Command(Command::TERMINATE, id));
    }

    void BrokerInterface::run(Broker &broker, const Command &command)
    {
        switch (command.type)
        {
        case Command::BEGIN:
            switch (command.args.size())
            {
            case 1:
                broker.begin(command.id, command.args[0]);
                break;
            case 3:
                broker.begin(command.id, command.args[0], ResourceLimits(
                                 boost::lexical_cast<std::uint64_t>(command.args[1]),
                                 boost::lexical_cast<std::uint64_t>(command.args[2])));
                break;
            default:
                BOOST_ASSERT(false);
            }
            break;
        case Command::SEND:
            BOOST_ASSERT(command.args.size() == 1);
            broker.send(command.id, command.args[0]);
            break;
        case Command::END:
            switch (command.args.size())
            {
            case 0:
                break;
                send(broker.end(command.id));
            case 1:
                send(broker.end(command.id, boost::lexical_cast<bool>(command.args[0])));
                break;
            default:
                BOOST_ASSERT(false);
            }
            break;
        case Command::TERMINATE:
            BOOST_ASSERT(command.args.size() == 0);
            break;
        }
    }
}}}}}
