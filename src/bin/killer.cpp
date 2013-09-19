#include <yandex/contest/invoker/flowctl/game/KillerImpl.hpp>
#include <yandex/contest/invoker/flowctl/game/KillerStreamInterface.hpp>

#include <yandex/contest/detail/LogHelper.hpp>
#include <yandex/contest/system/Trace.hpp>

#include <bunsan/config/input_archive.hpp>

#include <boost/assert.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace yandex::contest::invoker::flowctl::game;

KillerImpl::Options readOptions(const boost::filesystem::path &path)
{
    boost::property_tree::ptree ptree;
    boost::property_tree::read_json(path.c_str(), ptree);
    KillerImpl::Options options;
    bunsan::config::input_archive<boost::property_tree::ptree>::load_from_ptree(options, ptree);
    return options;
}

int main(int argc, char *argv[])
{
    try
    {
        yandex::contest::system::Trace::handle(SIGABRT);
        BOOST_ASSERT(argc == 1 + 1);
        KillerImpl killer(readOptions(argv[1]));
        KillerStreamInterface iface(std::cin, std::cout);
        for (;;)
            iface.runOnce(killer);
    }
    catch (EndOfFileError &e)
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
