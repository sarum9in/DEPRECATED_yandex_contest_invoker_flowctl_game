#include <yandex/contest/invoker/flowctl/game/KillerInterface.hpp>

#include <yandex/contest/detail/LogHelper.hpp>

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
        run(killer, command);
        STREAM_INFO << "Command " << STREAM_OBJECT(command) << " was successfully executed.";
    }

    void KillerInterface::run(Killer &killer, const Command &command)
    {
        switch (command.type)
        {
        case Command::FREEZE:
            send(killer.freeze(command.id));
            break;
        case Command::UNFREEZE:
            send(killer.unfreeze(command.id));
            break;
        case Command::TERMINATE:
            send(killer.terminate(command.id));
            break;
        }
    }
}}}}}
