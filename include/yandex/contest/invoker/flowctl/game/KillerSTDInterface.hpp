#pragma once

#include <iostream>

#include "yandex/contest/invoker/flowctl/game/KillerInterface.hpp"

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    /*!
     * \brief Binary interface using std::cin/std::cout.
     */
    class KillerSTDInterface: public KillerInterface
    {
    public:
        KillerSTDInterface()=default;

    protected:
        virtual void send(const Command &command) override;
        virtual void send(const Status &status) override;

        virtual Command recvCommand() override;
        virtual Status recvStatus() override;
    };
}}}}}
