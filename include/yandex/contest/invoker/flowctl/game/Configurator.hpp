#pragma once

#include "yandex/contest/invoker/All.hpp"

#include "yandex/contest/invoker/flowctl/game/BrokerImpl.hpp"
#include "yandex/contest/invoker/flowctl/game/Tokenizer.hpp"

#include "yandex/contest/StreamEnum.hpp"

#include <vector>
#include <string>
#include <limits>
#include <memory>

#include <boost/noncopyable.hpp>
#include <boost/assert.hpp>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    struct Configurator
    {
        ContainerPointer container;
        ProcessGroupPointer processGroup;
        ProcessPointer broker, killer, judge;
        std::vector<ProcessPointer> solutions;
        BrokerImpl::Options::Protocol protocol = BrokerImpl::Options::Protocol::TEXT;
        boost::filesystem::path killerLog, judgeLog, brokerLog;
        boost::filesystem::path tokenizer;
        Broker::ResourceLimits defaultResourceLimits;

        /*!
         * \brief Setup killer, judge, broker and solutions.
         *
         * \param broker create config for broker at specified path (not accessible)
         *
         * \note Streams, arguments, killer's ownerId will be overwritten.
         *
         * \note Broker's, judge's and solutions ownerIds should be set before.
         *
         * \warning This function take effect only once.
         * Next calls will cause undefined behavior.
         */
        void configure(const boost::filesystem::path &brokerConfig,
                       const boost::filesystem::path &killerConfig);

        /// configure(/*unspecified probably unique paths*/)
        void configure();

    private:
        bool configured_ = false;
    };
}}}}}
