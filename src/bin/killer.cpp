#include "yandex/contest/invoker/flowctl/game/KillerSTDInterface.hpp"

#include "yandex/contest/config/InputArchive.hpp"

#include <boost/assert.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace config = yandex::contest::config;
using namespace yandex::contest::invoker::flowctl::game;

Killer::Options readOptions(const boost::filesystem::path &path)
{
    boost::property_tree::ptree ptree;
    boost::property_tree::read_json(path.c_str(), ptree);
    Killer::Options options;
    config::InputArchive<boost::property_tree::ptree>::loadFromPtree(options, ptree);
    return options;
}

int main(int argc, char *argv[])
{
    BOOST_ASSERT(argc == 1 + 1);
    Killer killer(readOptions(argv[1]));
    KillerSTDInterface iface;
    try
    {
        for (;;)
            iface.runOnce(killer);
    }
    catch (EndOfFileError &e)
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
