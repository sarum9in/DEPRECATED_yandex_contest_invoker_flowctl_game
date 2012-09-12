#include "yandex/contest/invoker/flowctl/game/KillerImpl.hpp"

#include "yandex/contest/system/unistd/Operations.hpp"

#include "yandex/contest/system/cgroup/Freezer.hpp"

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    KillerImpl::KillerImpl(const Options &options):
        thisControlGroup_(system::cgroup::ControlGroup::getControlGroup(system::unistd::getpid())),
        parentControlGroup_(thisControlGroup_.parent()),
        pattern_(options.pattern)
    {
        protect_.insert(thisControlGroup_.name().string());
        for (const Id id: options.protect)
            protect_.insert(rawIdToCgroup(id));
    }

#define YANDEX_FLOWCTL_KILLER(FUNCTION, ACT) \
    KillerImpl::Status KillerImpl::FUNCTION(const Id &id) \
    { \
        const ControlGroupId cid = rawIdToCgroup(id); \
        if (protect_.find(cid) != protect_.end()) \
            return Status::PROTECTED; \
        system::cgroup::ControlGroup cg(parentControlGroup_.attachChild(cid)); \
        ACT; \
        return Status::OK; \
    }

    YANDEX_FLOWCTL_KILLER(freeze, system::cgroup::Freezer(cg).freeze())

    YANDEX_FLOWCTL_KILLER(unfreeze, system::cgroup::Freezer(cg).unfreeze())

    YANDEX_FLOWCTL_KILLER(terminate, cg.terminate())

    KillerImpl::ControlGroupId KillerImpl::rawIdToCgroup(const Id &id) const
    {
        return str(boost::format(pattern_) % id);
    }
}}}}}
