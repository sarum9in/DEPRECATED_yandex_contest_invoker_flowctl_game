#pragma once

#include <string>

#include <boost/noncopyable.hpp>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    class Killer: private boost::noncopyable
    {
    public:
        typedef std::size_t Id;

        enum class Status
        {
            OK,
            PROTECTED
        };

    public:
        virtual Status freeze(const Id &id)=0;
        virtual Status unfreeze(const Id &id)=0;
        virtual Status terminate(const Id &id)=0;

        virtual ~Killer();
    };
}}}}}
