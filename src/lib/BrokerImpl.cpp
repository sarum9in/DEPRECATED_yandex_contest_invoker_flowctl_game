#include "yandex/contest/invoker/flowctl/game/BrokerImpl.hpp"
#include "yandex/contest/invoker/flowctl/game/KillerStreamInterface.hpp"
#include "yandex/contest/invoker/flowctl/game/SharedTokenizer.hpp"

#include "yandex/contest/SystemError.hpp"

#include "yandex/contest/detail/LogHelper.hpp"

#include "yandex/contest/system/unistd/Operations.hpp"

#include <boost/format.hpp>

#include <fcntl.h>
#include <dlfcn.h>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    void setnonblock(const int fd)
    {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags < 0)
            BOOST_THROW_EXCEPTION(SystemError("fcntl") << system::unistd::info::fd(fd));
        if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
            BOOST_THROW_EXCEPTION(SystemError("fcntl") << system::unistd::info::fd(fd));
    }

    BrokerImpl::IOInterface::IOInterface(const Process &process):
        inputBuf(process.out, std::ios::in),
        outputBuf(process.in, std::ios::out),
        input(&inputBuf), output(&outputBuf) {}

    BrokerImpl::BrokerImpl(const Options &options):
        defaultResourceLimits_(options.defaultResourceLimits),
        defaultTokenizerArgument(options.defaultTokenizerArgument),
        tokenizerLibrary_(str(boost::format("lib%1%.so") % options.tokenizer.string()),
                          RTLD_NOW | RTLD_LOCAL),
        sharedTokenizerFactory_(tokenizerLibrary_),
        killerIO_(options.killer),
        killer_(new KillerStreamInterface(killerIO_.input, killerIO_.output)),
        solutions_(options.solutions.begin(), options.solutions.end())
    {
        for (std::size_t i = 0; i < solutions_.size(); ++i)
        {
            setnonblock(solutions_[i].process.in);
            setnonblock(solutions_[i].process.out);
            solutions_[i].resourceLimits = defaultResourceLimits_;
        }
    }

    void BrokerImpl::begin(const SolutionId id)
    {
        begin(id, defaultTokenizerArgument);
    }

    void BrokerImpl::begin(const SolutionId id,
                           const TokenizerArgument &tokenizerArgument)
    {
        begin(id, tokenizerArgument, defaultResourceLimits_);
    }

    void BrokerImpl::begin(const SolutionId id,
                           const TokenizerArgument &tokenizerArgument,
                           const ResourceLimits &resourceLimits)
    {
        Solution &sol = solution(id);
        BOOST_ASSERT_MSG(!sol.terminated, "Solution was already terminated!");
        BOOST_ASSERT_MSG(!sol.tokenizer, "Begin was already called!");
        sol.tokenizer = sharedTokenizerFactory_.instance(tokenizerArgument);
        sol.tokenizerStatus = Tokenizer::Status::CONTINUE;
        sol.result.clear();
        sol.resourceLimits = resourceLimits;
        {
            const Tokenizer::Result result = (*sol.tokenizer)(sol.outbuf);
            sol.tokenizerStatus = result.status;
            switch (result.status)
            {
            case Tokenizer::Status::FAIL:
                // nothing to do
                break;
            case Tokenizer::Status::ACCEPTED:
                {
                    const auto iter = sol.outbuf.begin() + result.accepted;
                    sol.result.append(sol.outbuf.begin(), iter);
                    sol.outbuf.erase(sol.outbuf.begin(), iter);
                }
                break;
            case Tokenizer::Status::CONTINUE:
                sol.result.append(sol.outbuf.begin(), sol.outbuf.end());
                sol.outbuf.clear();
                break;
            }
        }
    }

    void BrokerImpl::send(const SolutionId id, const std::string &msg)
    {
        Solution &sol = solution(id);
        BOOST_ASSERT_MSG(!sol.terminated, "Solution was already terminated!");
        BOOST_ASSERT_MSG(sol.tokenizer, "Begin was not called!");
        sol.inbuf.insert(sol.inbuf.end(), msg.begin(), msg.end());
    }

    BrokerImpl::Result BrokerImpl::end(const SolutionId id, const bool discardRemaining)
    {
        Solution &sol = solution(id);
        BOOST_ASSERT_MSG(!sol.terminated, "Solution was already terminated!");
        BOOST_ASSERT_MSG(sol.tokenizer, "Begin was not called!");
        if (sol.tokenizerStatus == Tokenizer::Status::CONTINUE)
            communicate(id);
        if (discardRemaining)
            sol.inbuf.clear();
        Result result;
        switch (sol.tokenizerStatus)
        {
        case Tokenizer::Status::FAIL:
            result.status = Result::Status::TOKENIZER_ERROR;
            if (sol.tokenizer->what())
                result.data = sol.tokenizer->what();
            break;
        case Tokenizer::Status::CONTINUE:
            result.status = Result::Status::EOF_ERROR;
            break;
        case Tokenizer::Status::ACCEPTED:
            result.status = Result::Status::OK;
            result.data = sol.result;
            break;
        }
        // FIXME check resource usage
        return result;
    }

    void BrokerImpl::terminate(const SolutionId id)
    {
        if (!solutions_[id].terminated)
        {
            if (killer_->terminate(solutions_[id].process.id) != Killer::Status::OK)
                STREAM_ERROR << "Unable to terminate solution id = " << id << ".";
            solutions_[id].terminated = true;
            solutions_[id].tokenizer.reset();
            solutions_[id].inbuf.clear();
            solutions_[id].outbuf.clear();
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
