#include "yandex/contest/invoker/flowctl/game/BrokerImpl.hpp"
#include "yandex/contest/invoker/flowctl/game/BrokerTextInterface.hpp"

#include "yandex/contest/config/InputArchive.hpp"

#include <boost/assert.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace yac = yandex::contest::config;
namespace yag = yandex::contest::invoker::flowctl::game;

yag::BrokerImpl::Options readOptions(const boost::filesystem::path &path)
{
    boost::property_tree::ptree ptree;
    boost::property_tree::read_json(path.c_str(), ptree);
    yag::BrokerImpl::Options options;
    yac::InputArchive<boost::property_tree::ptree>::loadFromPtree(options, ptree);
    return options;
}

int main(int argc, char *argv[])
{
    BOOST_ASSERT(argc == 1 + 1);
    const yag::BrokerImpl::Options options = readOptions(argv[1]);
    yag::BrokerImpl broker(options);
    yag::BrokerImpl::IOInterface ioIface(options.judge);
    std::unique_ptr<yag::BrokerInterface> iface;
    switch (options.protocol)
    {
    case yag::BrokerImpl::Options::Protocol::TEXT:
        iface.reset(new yag::BrokerTextInterface(ioIface.input, ioIface.output));
        break;
    default:
        std::cerr << "Unsupported protocol " << options.protocol << ".";
        return 1;
    }
    try
    {
        for (;;)
            iface->runOnce(broker);
    }
    catch (yag::EndOfFileError &e)
    {
        // it's OK
        // does nothing
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
