#pragma once

#include <yandex/contest/invoker/flowctl/game/BrokerInterace.hpp>

#include <istream>
#include <ostream>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    /// \note Stores references to std::iostreams.
    class BrokerTextInterface: public BrokerInterface
    {
    public:
        BrokerTextInterface(std::istream &input, std::ostream &output);

    protected:
        virtual void send(const Command &command) override;
        virtual void send(const Result &result) override;

        virtual Command recvCommand() override;
        virtual Result recvResult() override;

    private:
        std::istream &input_;
        std::ostream &output_;
    };
}}}}}
