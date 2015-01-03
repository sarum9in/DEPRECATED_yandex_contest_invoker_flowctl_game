#include <yandex/contest/invoker/flowctl/game/KillerImpl.hpp>

#include <yandex/contest/detail/LogHelper.hpp>

#include <yandex/contest/system/cgroup/Freezer.hpp>
#include <yandex/contest/system/cgroup/MultipleControlGroup.hpp>
#include <yandex/contest/system/cgroup/SingleControlGroup.hpp>
#include <yandex/contest/system/cgroup/Termination.hpp>
#include <yandex/contest/system/unistd/Operations.hpp>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    KillerImpl::KillerImpl(const Options &options):
        thisControlGroup_(system::cgroup::MultipleControlGroup::forSelf()),
        parentControlGroup_(thisControlGroup_->parent()),
        pattern_(options.pattern)
    {
        for (const Id id: options.unprotected)
            unprotected_.insert(rawIdToCgroup(id));
        unprotected_.erase(
            system::cgroup::SingleControlGroup::forSelf("freezer")->
            controlGroup().
            filename().
            string()
        );
    }

#define YANDEX_FLOWCTL_KILLER(FUNCTION, ACT) \
    KillerImpl::Status KillerImpl::FUNCTION(const Id &id) \
    { \
        STREAM_TRACE << "Attempt to " << __func__ << " id = " << id << "."; \
        const ControlGroupId cid = rawIdToCgroup(id); \
        if (unprotected_.find(cid) == unprotected_.end()) \
        { \
            STREAM_TRACE << "id = " << id << ", status = PROTECTED."; \
            return Status::PROTECTED; \
        } \
        try \
        { \
            const system::cgroup::ControlGroupPointer cg = \
                parentControlGroup_->attachChild(cid); \
            ACT; \
        } \
        catch (std::exception &e) \
        { \
            STREAM_TRACE << "Error: " << e.what() << "."; \
            return Status::ERROR; \
        } \
        STREAM_TRACE << "id = " << id << ", status = OK."; \
        return Status::OK; \
    }

    YANDEX_FLOWCTL_KILLER(freeze, system::cgroup::Freezer(cg).freeze())

    YANDEX_FLOWCTL_KILLER(unfreeze, system::cgroup::Freezer(cg).unfreeze())

    YANDEX_FLOWCTL_KILLER(terminate, system::cgroup::terminate(cg))

    KillerImpl::ControlGroupId KillerImpl::rawIdToCgroup(const Id &id) const
    {
        return str(boost::format(pattern_) % id);
    }
}}}}}
