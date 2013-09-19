#pragma once

#include <yandex/contest/invoker/flowctl/game/KillerInterface.hpp>

#include <istream>
#include <ostream>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    /*!
     * \brief Binary interface using iostreams.
     *
     * \note Stores references to given iostreams.
     */
    class KillerStreamInterface: public KillerInterface
    {
    public:
        KillerStreamInterface(std::istream &input, std::ostream &output);

    protected:
        virtual void send(const Command &command) override;
        virtual void send(const Status &status) override;

        virtual Command recvCommand() override;
        virtual Status recvStatus() override;

    private:
        std::istream &input_;
        std::ostream &output_;
    };
}}}}}
