#include "yandex/contest/invoker/flowctl/game/Killer.hpp"

#include "yandex/contest/system/unistd/Operations.hpp"

#include "yandex/contest/system/cgroup/Freezer.hpp"

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    Killer::Killer(const Options &options):
        thisControlGroup_(system::cgroup::ControlGroup::getControlGroup(system::unistd::getpid())),
        parentControlGroup_(thisControlGroup_.parent()),
        pattern_(options.pattern)
    {
        protect_.insert(thisControlGroup_.name().string());
        for (const Id id: options.protect)
            protect_.insert(rawIdToCgroup(id));
    }

    Killer::Status Killer::freeze(const Id &id) const
    {
        const ControlGroupId cid = rawIdToCgroup(id);
        if (protect_.find(cid) != protect_.end())
            return Status::PROTECTED;
        system::cgroup::ControlGroup cg(parentControlGroup_.attachChild(cid));
        const system::cgroup::Freezer freezer(cg);
        freezer.freeze();
        return Status::OK;
    }

    Killer::Status Killer::unfreeze(const Id &id) const
    {
        const ControlGroupId cid = rawIdToCgroup(id);
        if (protect_.find(cid) != protect_.end())
            return Status::PROTECTED;
        system::cgroup::ControlGroup cg(parentControlGroup_.attachChild(cid));
        const system::cgroup::Freezer freezer(cg);
        freezer.unfreeze();
        return Status::OK;
    }

    Killer::Status Killer::terminate(const Id &id) const
    {
        const ControlGroupId cid = rawIdToCgroup(id);
        if (protect_.find(cid) != protect_.end())
            return Status::PROTECTED;
        system::cgroup::ControlGroup cg(parentControlGroup_.attachChild(cid));
        cg.terminate();
        return Status::OK;
    }

    Killer::ControlGroupId Killer::rawIdToCgroup(const Id &id) const
    {
        return str(boost::format(pattern_) % id);
    }
}}}}}
