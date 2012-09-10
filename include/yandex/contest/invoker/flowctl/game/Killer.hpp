#pragma once

#include "yandex/contest/system/cgroup/ControlGroup.hpp"

#include <string>
#include <unordered_set>

#include <boost/noncopyable.hpp>
#include <boost/format.hpp>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    class Killer: private boost::noncopyable
    {
    public:
        typedef std::size_t Id;
        typedef std::string ControlGroupId;

        enum class Status
        {
            OK,
            PROTECTED
        };

        struct Options
        {
            /*!
             * \brief Do not operate on these processes.
             *
             * \note Killer's process is always protected.
             */
            std::unordered_set<Id> protect;

            /// The was of translation ids into cgroup names.
            boost::format pattern{"id_%1%"};
        };

    public:
        explicit Killer(const Options &options);

        Status freeze(const Id &id) const;
        Status unfreeze(const Id &id) const;
        Status terminate(const Id &id) const;

    private:
        ControlGroupId rawIdToCgroup(const Id &id) const;

    private:
        system::cgroup::ControlGroup thisControlGroup_, parentControlGroup_;
        std::unordered_set<ControlGroupId> protect_;
        boost::format pattern_;
    };
}}}}}
