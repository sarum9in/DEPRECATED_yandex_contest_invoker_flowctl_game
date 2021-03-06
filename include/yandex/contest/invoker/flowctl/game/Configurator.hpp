#pragma once

#include <yandex/contest/invoker/flowctl/game/BrokerImpl.hpp>
#include <yandex/contest/invoker/flowctl/game/Tokenizer.hpp>

#include <yandex/contest/invoker/All.hpp>

#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    struct Configurator
    {
        template <typename Archive>
        void serialize(Archive &ar, const unsigned int)
        {
            /// \note pointer serialization is not supported
            ar & BOOST_SERIALIZATION_NVP(protocol);
            ar & BOOST_SERIALIZATION_NVP(killerLog);
            ar & BOOST_SERIALIZATION_NVP(judgeLog);
            ar & BOOST_SERIALIZATION_NVP(brokerLog);
            ar & BOOST_SERIALIZATION_NVP(tokenizer);
            ar & BOOST_SERIALIZATION_NVP(defaultResourceLimits);
            ar & BOOST_SERIALIZATION_NVP(defaultTokenizerArgument);
        }

        ContainerPointer container;
        ProcessGroupPointer processGroup;
        ProcessPointer broker, killer, judge;
        std::vector<ProcessPointer> solutions;
        BrokerImpl::Options::Protocol protocol = BrokerImpl::Options::Protocol::TEXT;
        boost::filesystem::path killerLog, judgeLog, brokerLog;
        boost::filesystem::path tokenizer;
        Broker::ResourceLimits defaultResourceLimits;
        Broker::TokenizerArgument defaultTokenizerArgument;

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
