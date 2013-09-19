#include <yandex/contest/invoker/flowctl/game/BrokerImpl.hpp>

#include <yandex/contest/SystemError.hpp>

#include <yandex/contest/detail/LogHelper.hpp>

#include <yandex/contest/system/unistd/Operations.hpp>

#include <algorithm>
#include <chrono>

#include <cstdio>
#include <cerrno>

#include <sys/epoll.h>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    namespace unistd = system::unistd;

    typedef std::chrono::system_clock Clock;
    typedef Clock::duration Duration;
    typedef Clock::time_point TimePoint;

    BrokerImpl::Result::Status BrokerImpl::communicate(const SolutionId id)
    {
        STREAM_TRACE << "id = " << id << ".";
        Solution &sol = solution(id);
        BOOST_ASSERT_MSG(sol.tokenizer, "Begin was not called!");
        const TimePoint untilPoint = Clock::now() + sol.resourceLimits.realTimeLimit;
        const unistd::Descriptor epfd = unistd::epoll_create1();
        constexpr std::size_t MAXEVENTS = 2;
        struct epoll_event ev, events[MAXEVENTS];
        ev.events = EPOLLOUT;
        ev.data.fd = sol.process.in;
        unistd::epoll_ctl_add(epfd.get(), sol.process.in, ev);
        ev.events = EPOLLIN;
        ev.data.fd = sol.process.out;
        unistd::epoll_ctl_add(epfd.get(), sol.process.out, ev);
        STREAM_TRACE << "Starting communicate loop...";
        while (sol.tokenizerStatus == Tokenizer::Status::CONTINUE)
        {
            STREAM_TRACE << "Waiting for event(s)...";
            // TODO the last argument may be used in RTL control
            std::size_t nfds = 0;
            TimePoint now = Clock::now();
            while (!nfds && now < untilPoint)
            {
                // FIXME hardcode here
                nfds = unistd::epoll_wait(epfd.get(), events, std::chrono::milliseconds(100));
                now = Clock::now();
            }
            // If event has occurred we do not treat it as RTLE.
            if (!nfds && now >= untilPoint)
                return Result::Status::REAL_TIME_LIMIT_EXCEEDED;
            STREAM_TRACE << "Found " << nfds << " events.";
            for (std::size_t i = 0; i < nfds; ++i)
            {
                if (events[i].data.fd == sol.process.in)
                {
                    STREAM_TRACE << "Event at write end of pipe (fd = " << sol.process.in << ")...";
                    if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP))
                    {
                        if (events[i].events & EPOLLERR)
                            STREAM_TRACE << "Error at " << sol.process.in << ".";
                        if (events[i].events & EPOLLHUP)
                            STREAM_TRACE << "Hup at " << sol.process.in << ".";
                        STREAM_TRACE << "Deleting " << sol.process.in << " from epoll.";
                        system::unistd::epoll_ctl_del(epfd.get(), sol.process.in);
                        continue;
                    }
                    BOOST_ASSERT_MSG(events[i].events & EPOLLOUT, "Unexpected event.");
                    STREAM_TRACE << "May write into " << sol.process.in << ".";
                    if (sol.tokenizerStatus != Tokenizer::Status::CONTINUE)
                    {
                        STREAM_TRACE << "Nothing to write into " << sol.process.in << ".";
                        continue; // TODO may be break?
                    }
                    char buf[BUFSIZ];
                    STREAM_TRACE << "Starting write loop (fd = " << sol.process.in << ").";
                    while (!sol.inbuf.empty())
                    {
                        const std::size_t bsize = std::min((size_t)BUFSIZ, sol.inbuf.size());
                        std::copy_n(sol.inbuf.begin(), bsize, buf);
                        STREAM_TRACE << "Attempt to write " << bsize <<
                                        " bytes into " << sol.process.in << ".";
                        const ssize_t size = write(sol.process.in, buf, bsize);
                        if (size < 0)
                        {
                            if (errno == EAGAIN)
                            {
                                STREAM_TRACE << "Unable to write more into " <<
                                                sol.process.in << ".";
                                break;
                            }
                            else
                            {
                                BOOST_THROW_EXCEPTION(SystemError("write") <<
                                                      unistd::info::fd(sol.process.in));
                            }
                        }
                        STREAM_TRACE << "Written " << size << " bytes into " <<
                                        sol.process.in << ".";
                        sol.inbuf.erase(sol.inbuf.begin(), sol.inbuf.begin() + size);
                    }
                    if (sol.inbuf.empty())
                    {
                        STREAM_TRACE << "inbuf is empty (fd = " << sol.process.in << "), deleting.";
                        system::unistd::epoll_ctl_del(epfd.get(), sol.process.in);
                    }
                }
                else if (events[i].data.fd == sol.process.out)
                {
                    STREAM_TRACE << "Event at read end of pipe (fd = " << sol.process.out << ")...";
                    // Even if error occurred, we can read available data.
                    if (!(events[i].events & EPOLLIN))
                    {
                        if (events[i].events & EPOLLERR)
                            STREAM_TRACE << "Error at " << sol.process.out << ".";
                        if (events[i].events & EPOLLHUP)
                            STREAM_TRACE << "Hup at " << sol.process.out << ".";
                        STREAM_TRACE << "EOF at " << sol.process.out << ".";
                        return Result::Status::OK;
                    }
                    BOOST_ASSERT_MSG(events[i].events & EPOLLIN, "Unexpected event.");
                    STREAM_TRACE << "May read from " << sol.process.out << ".";
                    char buf[BUFSIZ];
                    ssize_t size;
                    STREAM_TRACE << "Starting read loop (fd = " << sol.process.out << ").";
                    while (sol.tokenizerStatus == Tokenizer::Status::CONTINUE &&
                           (size = read(sol.process.out, buf, BUFSIZ)))
                    {
                        if (size < 0)
                        {
                            if (errno == EAGAIN)
                            {
                                STREAM_TRACE << "Unable to read more from " <<
                                                sol.process.out << ".";
                                break;
                            }
                            else
                            {
                                BOOST_THROW_EXCEPTION(SystemError("read") <<
                                                      unistd::info::fd(sol.process.out));
                            }
                        }
                        STREAM_TRACE << "Read " << size << " bytes from " <<
                                        sol.process.out << ".";
                        const Tokenizer::Result result = (*sol.tokenizer)(buf, size);
                        sol.tokenizerStatus = result.status;
                        STREAM_TRACE << "Tokenizer::Status = " << result.status << ".";
                        switch (result.status)
                        {
                        case Tokenizer::Status::FAIL:
                            unistd::epoll_ctl_del(epfd.get(), sol.process.out);
                            break;
                        case Tokenizer::Status::CONTINUE:
                            sol.result.append(buf, size);
                            break;
                        case Tokenizer::Status::ACCEPTED:
                            sol.result.append(buf, result.accepted);
                            sol.outbuf.insert(sol.outbuf.end(), buf + result.accepted, buf + size);
                            break;
                        }
                    }
                }
            }
        }
        return Result::Status::OK;
    }
}}}}}
