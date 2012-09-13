#include "yandex/contest/invoker/flowctl/game/BrokerImpl.hpp"

#include "yandex/contest/SystemError.hpp"

#include "yandex/contest/system/unistd/Operations.hpp"

#include <algorithm>

#include <cerrno>

#include <sys/epoll.h>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    namespace unistd = system::unistd;

    void BrokerImpl::communicate(const SolutionId id)
    {
        Solution &sol = solution(id);
        BOOST_ASSERT_MSG(!sol.terminated, "Solution was already terminated!");
        BOOST_ASSERT_MSG(sol.tokenizer, "Begin was not called!");
        const unistd::Descriptor epfd = unistd::epoll_create1();
        constexpr std::size_t MAXEVENTS = 2;
        struct epoll_event ev, events[MAXEVENTS];
        ev.events = EPOLLIN;
        ev.data.fd = sol.process.in;
        unistd::epoll_ctl_add(epfd.get(), sol.process.in, ev);
        ev.events = EPOLLOUT;
        ev.data.fd = sol.process.out;
        unistd::epoll_ctl_add(epfd.get(), sol.process.out, ev);
        while (sol.tokenizerStatus == Tokenizer::Status::CONTINUE)
        {
            // TODO the last argument may be used in RTL control
            const std::size_t nfds = unistd::epoll_wait(epfd.get(), events);
            for (std::size_t i = 0; i < nfds; ++i)
            {
                if (events[i].data.fd == sol.process.in)
                {
                    // may write
                    if (sol.tokenizerStatus != Tokenizer::Status::CONTINUE)
                        continue; // TODO may be break?
                    constexpr std::size_t BUFSIZE = 1024;
                    char buf[BUFSIZE];
                    for (;;)
                    {
                        const std::size_t bsize = std::min(BUFSIZE, sol.inbuf.size());
                        std::copy_n(sol.inbuf.begin(), bsize, buf);
                        const ssize_t size = write(sol.process.in, buf, bsize);
                        if (size < 0)
                        {
                            if (errno == EAGAIN)
                                break;
                            else
                                BOOST_THROW_EXCEPTION(SystemError("write") <<
                                                      unistd::info::fd(sol.process.in));
                        }
                        sol.inbuf.erase(sol.inbuf.begin(), sol.inbuf.begin() + size);
                    }
                }
                else if (events[i].data.fd == sol.process.out)
                {
                    // may read
                    constexpr std::size_t BUFSIZE = 1024;
                    char buf[BUFSIZE];
                    ssize_t size;
                    while (sol.tokenizerStatus == Tokenizer::Status::CONTINUE &&
                           (size = read(sol.process.out, buf, BUFSIZE)))
                    {
                        if (size < 0)
                        {
                            if (errno == EAGAIN)
                                break;
                            else
                                BOOST_THROW_EXCEPTION(SystemError("read") <<
                                                      unistd::info::fd(sol.process.out));
                        }
                        const Tokenizer::Result result = (*sol.tokenizer)(buf, size);
                        sol.tokenizerStatus = result.status;
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
    }
}}}}}
