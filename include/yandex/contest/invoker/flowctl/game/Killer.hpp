#pragma once

#include <yandex/contest/invoker/Process.hpp>

#include <bunsan/stream_enum.hpp>

#include <boost/noncopyable.hpp>

#include <string>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    class Killer: private boost::noncopyable
    {
    public:
        typedef Process::Id Id;

        BUNSAN_INCLASS_STREAM_ENUM_CLASS(Status,
        (
            OK,
            PROTECTED,
            ERROR
        ))

    public:
        virtual Status freeze(const Id &id)=0;
        virtual Status unfreeze(const Id &id)=0;
        virtual Status terminate(const Id &id)=0;

        virtual ~Killer();
    };
}}}}}
