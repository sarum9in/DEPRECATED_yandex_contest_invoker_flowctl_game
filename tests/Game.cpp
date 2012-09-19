#define BOOST_TEST_MODULE Game
#include <boost/test/unit_test.hpp>

#include "yandex/contest/tests/BoostExecTest.hpp"
#include "yandex/contest/tests/Environment.hpp"
#include "yandex/contest/tests/Utils.hpp"

#include "yandex/contest/invoker/tests/ContainerFixture.hpp"

#include "yandex/contest/system/unistd/access/Operations.hpp"

#include "yandex/contest/invoker/flowctl/game/Configurator.hpp"

#include <boost/filesystem/operations.hpp>

namespace ya = yandex::contest::invoker;
namespace yag = ya::flowctl::game;
namespace unistd = yandex::contest::system::unistd;

struct GameFixture: ContainerFixture
{
    GameFixture():
        broker(cfg.broker),
        killer(cfg.killer),
        judge(cfg.judge)
    {
        BOOST_REQUIRE_EQUAL(unistd::access::getEffectiveId().uid, 0);
        cfg.container = cnt;
        cfg.processGroup = pg;
        brokerLog = cnt->filesystem().keepInRoot(cfg.brokerLog = "/brokerLog");
        killerLog = cnt->filesystem().keepInRoot(cfg.killerLog = "/killerLog");
        judgeLog = cnt->filesystem().keepInRoot(cfg.judgeLog = "/judgeLog");
        broker = p(0, "yandex_contest_invoker_flowctl_game_broker");
        killer = p(1, "yandex_contest_invoker_flowctl_game_killer");
        cfg.tokenizer = "yandex_contest_invoker_flowctl_game_tokenizer_split_char";
    }

    void run()
    {
        cfg.configure();
        pgr = pg->synchronizedCall();
        verifyOK();
        BOOST_TEST_MESSAGE("Broker: <<<\n" << readData(brokerLog) << "\n>>> Broker");
        BOOST_TEST_MESSAGE("Killer: <<<\n" << readData(killerLog) << "\n>>> Killer");
        BOOST_TEST_MESSAGE("Judge: <<<\n" << readData(judgeLog) << "\n>>> Judge");
    }

    yag::Configurator cfg;
    boost::filesystem::path brokerLog, killerLog, judgeLog;
    ya::ProcessPointer &broker, &killer, &judge;
    ya::ProcessGroup::Result pgr;
};

BOOST_FIXTURE_TEST_SUITE(Game, GameFixture)

BOOST_AUTO_TEST_CASE(empty)
{
    judge = p(2, testsResourcesSourceDir / "judge.py");
    run();
}

BOOST_AUTO_TEST_SUITE_END()
