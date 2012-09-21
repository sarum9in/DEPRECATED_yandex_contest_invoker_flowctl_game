#include "yandex/contest/invoker/flowctl/game/BrokerInterace.hpp"

#include "yandex/contest/detail/LogHelper.hpp"

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    void BrokerInterface::runOnce(Broker &broker)
    {
        STREAM_INFO << "Waiting for next command...";
        const Command command = recvCommand();
        STREAM_INFO << "Received " << STREAM_OBJECT(command) << ", executing...";
        run(broker, command);
        STREAM_INFO << "Command " << STREAM_OBJECT(command) << " was successfully executed.";
    }

    void BrokerInterface::begin(const SolutionId id,
                                const TokenizerArgument &tokenizerArgument,
                                const ResourceLimits &resourceLimits)
    {
        send(Command(Command::BEGIN, id, tokenizerArgument,
                     resourceLimits.timeLimit.count(), resourceLimits.realTimeLimit.count()));
    }

    void BrokerInterface::begin(const SolutionId id,
                                const TokenizerArgument &tokenizerArgument)
    {
        send(Command(Command::BEGIN, id, tokenizerArgument));
    }

    void BrokerInterface::begin(const SolutionId id)
    {
        send(Command(Command::BEGIN, id));
    }

    void BrokerInterface::send(const SolutionId id, const std::string &msg)
    {
        send(Command(Command::SEND, id, msg));
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
            case 0:
                broker.begin(command.id);
                break;
            case 1:
                broker.begin(command.id, command.args[0]);
                break;
            case 3:
                broker.begin(command.id, command.args[0], ResourceLimits(
                             std::chrono::milliseconds(
                                 boost::lexical_cast<std::chrono::milliseconds::rep>(
                                     command.args[1])),
                             std::chrono::milliseconds(
                                 boost::lexical_cast<std::chrono::milliseconds::rep>(
                                     command.args[2]))));
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
