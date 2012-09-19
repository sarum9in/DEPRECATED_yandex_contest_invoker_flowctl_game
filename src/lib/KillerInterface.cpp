#include "yandex/contest/invoker/flowctl/game/KillerInterface.hpp"

#include "yandex/contest/detail/LogHelper.hpp"

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    KillerInterface::Status KillerInterface::freeze(const Id &id)
    {
        send(Command{Command::FREEZE, id});
        return recvStatus();
    }

    KillerInterface::Status KillerInterface::unfreeze(const Id &id)
    {
        send(Command{Command::UNFREEZE, id});
        return recvStatus();
    }

    KillerInterface::Status KillerInterface::terminate(const Id &id)
    {
        send(Command{Command::TERMINATE, id});
        return recvStatus();
    }

    void KillerInterface::runOnce(Killer &killer)
    {
        STREAM_INFO << "Waiting for next command...";
        const Command command = recvCommand();
        STREAM_INFO << "Received " << STREAM_OBJECT(command) << ", executing...";
        const Status status = run(killer, recvCommand());
        STREAM_INFO << "Command " << STREAM_OBJECT(command) <<
                       " was executed with status = " << status << ", sending...";
        send(status);
        STREAM_INFO << "Status " << status << " was successfully sent.";
    }

    KillerInterface::Status KillerInterface::run(Killer &killer, const Command &command)
    {
        switch (command.type)
        {
        case Command::FREEZE:
            return killer.freeze(command.id);
        case Command::UNFREEZE:
            return killer.unfreeze(command.id);
        case Command::TERMINATE:
            return killer.terminate(command.id);
        }
        BOOST_ASSERT(false);
        return Status::OK;
    }
}}}}}
