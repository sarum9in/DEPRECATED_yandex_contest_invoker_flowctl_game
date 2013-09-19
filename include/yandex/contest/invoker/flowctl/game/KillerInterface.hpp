#pragma once

#include <yandex/contest/invoker/flowctl/game/Error.hpp>
#include <yandex/contest/invoker/flowctl/game/Killer.hpp>

#include <bunsan/stream_enum.hpp>

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    class KillerInterface: public Killer
    {
    public:
        virtual Status freeze(const Id &id) override;
        virtual Status unfreeze(const Id &id) override;
        virtual Status terminate(const Id &id) override;

        void runOnce(Killer &killer);

    protected:
        struct Command
        {
            BUNSAN_INCLASS_STREAM_ENUM(Type,
            (
                FREEZE,
                UNFREEZE,
                TERMINATE
            ))

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
        void run(Killer &killer, const Command &command);
    };
}}}}}
