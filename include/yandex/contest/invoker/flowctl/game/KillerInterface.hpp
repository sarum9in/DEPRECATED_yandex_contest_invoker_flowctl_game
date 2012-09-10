#pragma once

#include "yandex/contest/invoker/flowctl/game/Killer.hpp"
#include "yandex/contest/invoker/flowctl/game/Error.hpp"

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/noncopyable.hpp>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    class KillerInterface: private boost::noncopyable
    {
    public:
        typedef Killer::Id Id;
        typedef Killer::Status Status;

    public:
        Status freeze(const Id &id);
        Status unfreeze(const Id &id);
        Status terminate(const Id &id);

        void runOnce(const Killer &killer);

        virtual ~KillerInterface();

    protected:
        struct Command
        {
            enum Type
            {
                FREEZE,
                UNFREEZE,
                TERMINATE
            };

            template <typename Archive>
            void serialize(Archive &ar, const unsigned int)
            {
                ar & BOOST_SERIALIZATION_NVP(type);
                ar & BOOST_SERIALIZATION_NVP(id);
            }

            Type type;
            Id id;
        };

    protected:
        virtual void send(const Command &command)=0;
        virtual void send(const Status &status)=0;

        virtual Command recvCommand()=0;
        virtual Status recvStatus()=0;

    private:
        static Status run(const Killer &killer, const Command &command);
    };
}}}}}
