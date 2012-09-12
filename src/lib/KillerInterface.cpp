#include "yandex/contest/invoker/flowctl/game/KillerInterface.hpp"

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
        send(run(killer, recvCommand()));
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
