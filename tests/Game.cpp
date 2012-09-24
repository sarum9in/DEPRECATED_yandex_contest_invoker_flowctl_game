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
        broker = pn(0, "broker", "yandex_contest_invoker_flowctl_game_broker");
        killer = pn(1, "killer", "yandex_contest_invoker_flowctl_game_killer");
        cfg.tokenizer = "yandex_contest_invoker_flowctl_game_tokenizer_split_char";
    }

    /// \note Judge is always added from source tree.
    ya::ProcessPointer setJudge(const boost::filesystem::path &path)
    {
        cnt->filesystem().push(path, "/" / path.filename(), {0, 0}, 0755);
        return judge = pn(2, "judge", "/" / path.filename());
    }

    template <typename ... Args>
    ya::ProcessPointer addSolution(Args &&...args)
    {
        const ya::ProcessPointer pp = p(std::forward<Args>(args)...);
        cfg.solutions.push_back(pp);
        return pp;
    }

    void run()
    {
        cfg.configure("/brokerConfig", "/killerConfig");
        pgr = pg->synchronizedCall();
        verifyPG(PGR::CompletionStatus::OK);
        for (std::size_t i = 0; i <= 2; ++i)
            verifyP(i, PR::CompletionStatus::OK);
        for (std::size_t i = 0; i < cfg.solutions.size(); ++i)
            infoP(i + 3);
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
    setJudge(testsResourcesSourceDir / "killer_judge.py");
    run();
}

BOOST_AUTO_TEST_CASE(kill_all)
{
    setJudge(testsResourcesSourceDir / "killer_judge.py");
    addSolution(3, "sleep", "5");
    addSolution(4, "sleep", "5");
    addSolution(5, "sleep", "5");
    run();
}

BOOST_AUTO_TEST_SUITE_END()
