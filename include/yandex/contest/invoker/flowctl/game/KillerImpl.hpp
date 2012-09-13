#pragma once

#include "yandex/contest/invoker/flowctl/game/Killer.hpp"

#include "yandex/contest/system/cgroup/ControlGroup.hpp"

#include <string>
#include <unordered_set>

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/noncopyable.hpp>
#include <boost/format.hpp>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    class KillerImpl: public Killer
    {
    public:
        typedef std::string ControlGroupId;

    public:
        struct Options
        {
            template <typename Archive>
            void serialize(Archive &ar, const unsigned int)
            {
                ar & BOOST_SERIALIZATION_NVP(unprotected);
                ar & BOOST_SERIALIZATION_NVP(pattern);
            }

            /*!
             * \brief Operate only with these processes.
             *
             * \note Killer's process will be erased from this set.
             */
            std::unordered_set<Id> unprotected;

            /// The was of translation ids into cgroup names.
            std::string pattern = "id_%1%";
        };

    public:
        explicit KillerImpl(const Options &options);

        virtual Status freeze(const Id &id) override;
        virtual Status unfreeze(const Id &id) override;
        virtual Status terminate(const Id &id) override;

    private:
        ControlGroupId rawIdToCgroup(const Id &id) const;

    private:
        system::cgroup::ControlGroup thisControlGroup_, parentControlGroup_;
        std::unordered_set<ControlGroupId> unprotected_;
        boost::format pattern_;
    };
}}}}}
