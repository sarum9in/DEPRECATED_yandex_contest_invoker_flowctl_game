#include "yandex/contest/invoker/flowctl/game/BrokerImpl.hpp"
#include "yandex/contest/invoker/flowctl/game/KillerStreamInterface.hpp"
#include "yandex/contest/invoker/flowctl/game/SharedTokenizer.hpp"

#include "yandex/contest/SystemError.hpp"

#include "yandex/contest/detail/LogHelper.hpp"

#include "yandex/contest/system/unistd/Operations.hpp"

#include <boost/scope_exit.hpp>

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
        tokenizerLibrary_(options.tokenizer, RTLD_NOW | RTLD_LOCAL),
        sharedTokenizerFactory_(tokenizerLibrary_),
        killerIO_(options.killer),
        killer_(new KillerStreamInterface(killerIO_.input, killerIO_.output)),
        solutions_(options.solutions.begin(), options.solutions.end())
    {
        for (std::size_t i = 0; i < solutions_.size(); ++i)
        {
            freeze(i);
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
        BOOST_ASSERT_MSG(!sol.tokenizer, "Begin was already called!");
        sol.tokenizer = sharedTokenizerFactory_.instance(tokenizerArgument);
        sol.tokenizerStatus = Tokenizer::Status::CONTINUE;
        sol.result.clear();
        sol.resourceLimits = resourceLimits;
        if (!sol.outbuf.empty())
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
        BOOST_ASSERT_MSG(sol.tokenizer, "Begin was not called!");
        sol.inbuf.insert(sol.inbuf.end(), msg.begin(), msg.end());
    }

    BrokerImpl::Result BrokerImpl::end(const SolutionId id, const bool discardRemaining)
    {
        STREAM_TRACE << "id = " << id << ", discardRemaining = " << discardRemaining << ".";
        Solution &sol = solution(id);
        BOOST_ASSERT_MSG(sol.tokenizer, "Begin was not called!");
        BOOST_SCOPE_EXIT_ALL(this, id)
        {
            solution(id).tokenizer.reset();
        };
        Result result;
        if (sol.tokenizerStatus == Tokenizer::Status::CONTINUE)
        {
            // Even if process has already terminated
            // some data may be available
            // from previous iterations.
            (void) unfreeze(id);
            result.status = communicate(id);
            (void) freeze(id);
        }
        if (discardRemaining)
            sol.inbuf.clear();
        if (result.status != Result::Status::OK)
            return result;
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
        return result;
    }

    void BrokerImpl::terminate(const SolutionId id)
    {
        Solution &sol = solution(id);
        if (killer_->terminate(sol.process.id) != Killer::Status::OK)
            STREAM_ERROR << "Unable to terminate solution id = " << id << ".";
        sol.tokenizer.reset();
        sol.inbuf.clear();
        sol.outbuf.clear();
    }

    BrokerImpl::Solution &BrokerImpl::solution(const SolutionId id)
    {
        BOOST_ASSERT(id < solutions_.size());
        Solution &sol = solutions_[id];
        return sol;
    }

    Killer::Status BrokerImpl::freeze(const SolutionId id)
    {
        STREAM_TRACE << "Attempt to freeze id = " << id << ".";
        const Killer::Status status = killer_->freeze(solution(id).process.id);
        BOOST_ASSERT_MSG(status != Killer::Status::PROTECTED, "Invalid configuration.");
        return status;
    }

    Killer::Status BrokerImpl::unfreeze(const SolutionId id)
    {
        STREAM_TRACE << "Attempt to unfreeze id = " << id << ".";
        const Killer::Status status = killer_->unfreeze(solution(id).process.id);
        BOOST_ASSERT_MSG(status != Killer::Status::PROTECTED, "Invalid configuration.");
        return status;
    }
}}}}}
