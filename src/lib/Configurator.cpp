#include "yandex/contest/invoker/flowctl/game/Configurator.hpp"
#include "yandex/contest/invoker/flowctl/game/KillerImpl.hpp"

#include "yandex/contest/config/OutputArchive.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    namespace
    {
        template <typename Options>
        void dumpOptions(const ContainerPointer &container,
                         const ProcessPointer &proc,
                         const Options &options,
                         const boost::filesystem::path &path)
        {
            boost::property_tree::ptree cfg;
            config::OutputArchive<boost::property_tree::ptree>::saveToPtree(options, cfg);
            boost::property_tree::write_json(
                container->filesystem().keepInRoot(path).string(), cfg);
            container->filesystem().setOwnerId(path, proc->ownerId());
            container->filesystem().setMode(path, 0400);
            proc->setArguments(proc->executable(), path);
        }
    }

    void Configurator::configure(const boost::filesystem::path &brokerConfig,
                                 const boost::filesystem::path &killerConfig)
    {
        if (!configured_)
        {
            BrokerImpl::Options brokerOptions;
            KillerImpl::Options killerOptions;
            brokerOptions.protocol = protocol;
            brokerOptions.tokenizer = tokenizer;
            brokerOptions.defaultResourceLimits = defaultResourceLimits;
            brokerOptions.defaultTokenizerArgument = defaultTokenizerArgument;
            broker->setStream(2, File(brokerLog, AccessMode::WRITE_ONLY));
            std::size_t brokerFD = 3;
            auto connect =
                [&](const ProcessPointer &proc, BrokerImpl::Process &cfg)
                {
                    cfg.id = proc->id();
                    Pipe in = processGroup->createPipe(), out = processGroup->createPipe();
                    proc->setStream(0, in.readEnd());
                    proc->setStream(1, out.writeEnd());
                    broker->setStream(cfg.in = brokerFD++, in.writeEnd());
                    broker->setStream(cfg.out = brokerFD++, out.readEnd());
                };
            connect(killer, brokerOptions.killer);
            killer->setStream(2, File(killerLog, AccessMode::WRITE_ONLY));
            killer->setOwnerId(system::unistd::access::Id(0, 0));
            connect(judge, brokerOptions.judge);
            judge->setStream(2, File(judgeLog, AccessMode::WRITE_ONLY));
            brokerOptions.solutions.resize(solutions.size());
            for (std::size_t i = 0; i < solutions.size(); ++i)
            {
                connect(solutions[i], brokerOptions.solutions[i]);
                killerOptions.unprotected.insert(solutions[i]->id());
                // Solution may crash, it should not affect broker and other processes.
                solutions[i]->setTerminateGroupOnCrash(false);
                // If broker exits before solution, it should be killed.
                solutions[i]->setGroupWaitsForTermination(false);
            }
            dumpOptions(container, broker, brokerOptions, brokerConfig);
            dumpOptions(container, killer, killerOptions, killerConfig);
            judge->setArguments(judge->executable(),
                                boost::lexical_cast<std::string>(solutions.size()));
            // the last command
            configured_ = true;
        }
    }

    void Configurator::configure()
    {
        configure("/" / boost::filesystem::unique_path(),
                  "/" / boost::filesystem::unique_path());
    }
}}}}}
