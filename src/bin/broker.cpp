#include <yandex/contest/invoker/flowctl/game/BrokerImpl.hpp>
#include <yandex/contest/invoker/flowctl/game/BrokerTextInterface.hpp>

#include <yandex/contest/detail/LogHelper.hpp>
#include <yandex/contest/system/Trace.hpp>

#include <bunsan/config/cast.hpp>

#include <boost/assert.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace yag = yandex::contest::invoker::flowctl::game;

yag::BrokerImpl::Options readOptions(const boost::filesystem::path &path)
{
    boost::property_tree::ptree ptree;
    boost::property_tree::read_json(path.c_str(), ptree);
    return bunsan::config::load<yag::BrokerImpl::Options>(ptree);
}

int main(int argc, char *argv[])
{
    try
    {
        yandex::contest::system::Trace::handle(SIGABRT);
        BOOST_ASSERT(argc == 1 + 1);
        const yag::BrokerImpl::Options options = readOptions(argv[1]);
        yag::BrokerImpl broker(options);
        yag::BrokerImpl::IOInterface ioIface(options.judge);
        std::unique_ptr<yag::BrokerInterface> iface;
        switch (options.protocol)
        {
        case yag::BrokerImpl::Options::Protocol::TEXT:
            iface.reset(
                new yag::BrokerTextInterface(ioIface.input, ioIface.output)
            );
            break;
        default:
            std::cerr << "Unsupported protocol " << options.protocol << ".";
            return 1;
        }
        for (;;)
            iface->runOnce(broker);
    }
    catch (yag::EndOfFileError &e)
    {
        STREAM_INFO << "EOF was reached, terminating.";
        STREAM_DEBUG << e.what();
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
