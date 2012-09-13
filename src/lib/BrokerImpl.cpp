#include "yandex/contest/invoker/flowctl/game/BrokerImpl.hpp"
#include "yandex/contest/invoker/flowctl/game/KillerStreamInterface.hpp"

#include "yandex/contest/detail/LogHelper.hpp"

#include <dlfcn.h>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    BrokerImpl::BrokerImpl(const Options &options):
        defaultResourceLimits_(options.defaultResourceLimits),
        tokenizerLibrary_(options.tokenizer, RTLD_LOCAL),
        killerIO_(options.killer),
        killer_(new KillerStreamInterface(killerIO_.input, killerIO_.output)),
        solutions_(options.solutions.begin(), options.solutions.end())
    {
        for (std::size_t i = 0; i < solutions_.size(); ++i)
        {
            solutions_[i].resourceLimits = defaultResourceLimits_;
        }
    }

    void BrokerImpl::begin(const SolutionId id, const std::string &tokenizerArgument)
    {
        begin(id, tokenizerArgument, defaultResourceLimits_);
    }

    void BrokerImpl::begin(const SolutionId id, const std::string &tokenizerArgument,
                       const ResourceLimits &resourceLimits)
    {
        Solution &sol = solution(id);
        BOOST_ASSERT_MSG(!sol.tokenizer, "Begin was already called!");
        // TODO sol.reset(new Tokenizer(tokenizerArgument))
        sol.resourceLimits = resourceLimits;
        // TODO process remaining data to tokenizer
    }

    void BrokerImpl::send(const SolutionId id, const std::string &msg)
    {
        Solution &sol = solution(id);
        BOOST_ASSERT_MSG(sol.tokenizer, "Begin was not called!");
        sol.inbuf.insert(sol.inbuf.end(), msg.begin(), msg.end());
    }

    BrokerImpl::Result BrokerImpl::end(const SolutionId id, const bool discardRemaining)
    {
        Solution &sol = solution(id);
        BOOST_ASSERT_MSG(sol.tokenizer, "Begin was not called!");
        // TODO wait for other unwritten data (if needed)
        if (discardRemaining)
            sol.inbuf.clear();
        Result result;
        // fill result
        return result;
    }

    void BrokerImpl::terminate(const SolutionId id)
    {
        if (!solutions_[id].terminated)
        {
            if (killer_->terminate(solutions_[id].process.id) != Killer::Status::OK)
                STREAM_ERROR << "Unable to terminate solution id = " << id << ".";
            solutions_[id].terminated = true;
        }
    }

    BrokerImpl::Solution &BrokerImpl::solution(const SolutionId id)
    {
        BOOST_ASSERT(id < solutions_.size());
        Solution &sol = solutions_[id];
        BOOST_ASSERT_MSG(!sol.terminated, "Was already terminated!");
        return sol;
    }
}}}}}
