#define BOOST_TEST_MODULE Game
#include <boost/test/unit_test.hpp>

#include <yandex/contest/invoker/flowctl/game/Configurator.hpp>
#include <yandex/contest/invoker/tests/ContainerFixture.hpp>
#include <yandex/contest/system/unistd/access/Operations.hpp>

#include <bunsan/testing/environment.hpp>
#include <bunsan/testing/exec_test.hpp>
#include <bunsan/testing/filesystem/read_data.hpp>

#include <boost/filesystem/operations.hpp>

#include <functional>

namespace ya = yandex::contest::invoker;
namespace yag = ya::flowctl::game;
namespace unistd = yandex::contest::system::unistd;

using namespace bunsan::testing;

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
        pushLibraries();
    }

    void pushLibraries()
    {
        push(dir::tests::resources::source() / "judge.py", {0, 0}, 0644);
    }

    boost::filesystem::path push(const boost::filesystem::path &path,
                                 const unistd::access::Id &id,
                                 const mode_t mode)
    {
        const boost::filesystem::path path_ = "/" / path.filename();
        cnt->filesystem().push(path, path_, id, mode);
        return path_;
    }

    /// \note Judge is always added from source tree.
    ya::ProcessPointer setJudge(const boost::filesystem::path &path)
    {
        push(path, {0, 0}, 0755);
        return judge = pn(2, "judge", "/" / path.filename());
    }

    template <typename ... Args>
    ya::ProcessPointer addSolution(Args &&...args)
    {
        const ya::ProcessPointer pp = p(std::forward<Args>(args)...);
        cfg.solutions.push_back(pp);
        return pp;
    }

    template <typename Arg0, typename ... Args>
    ya::ProcessPointer addSolutionCopy(Arg0 &&arg0,
                                       const boost::filesystem::path &executable,
                                       Args &&...args)
    {
        push(executable, {0, 0}, 0755);
        const ya::ProcessPointer pp = p(std::forward<Arg0>(arg0),
                                        "/" / executable.filename(),
                                        std::forward<Args>(args)...);
        cfg.solutions.push_back(pp);
        return pp;
    }

    void run_(const std::function<void (const std::size_t)> &checkSolution)
    {
        cfg.configure("/brokerConfig", "/killerConfig");
        pgr = pg->synchronizedCall();
        verifyPG(PGR::CompletionStatus::OK);
        for (std::size_t i = 0; i < 3; ++i)
            verifyP(i, PR::CompletionStatus::OK);
        for (std::size_t i = 0; i < cfg.solutions.size(); ++i)
        {
            BOOST_TEST_MESSAGE("Solution [" << i << "]:");
            checkSolution(i + 3);
        }
        BOOST_TEST_MESSAGE("Broker: <<<\n" << filesystem::read_data(brokerLog) << "\n>>> Broker");
        BOOST_TEST_MESSAGE("Killer: <<<\n" << filesystem::read_data(killerLog) << "\n>>> Killer");
        BOOST_TEST_MESSAGE("Judge: <<<\n" << filesystem::read_data(judgeLog) << "\n>>> Judge");
    }

    void runInfo()
    {
        run_([this](const std::size_t i){ infoP(i); });
    }

    void runOK()
    {
        run_([this](const std::size_t i){ verifyP(i); });
    }

    yag::Configurator cfg;
    boost::filesystem::path brokerLog, killerLog, judgeLog;
    ya::ProcessPointer &broker, &killer, &judge;
    ya::ProcessGroup::Result pgr;
};

BOOST_FIXTURE_TEST_SUITE(Game, GameFixture)

BOOST_AUTO_TEST_CASE(empty)
{
    setJudge(dir::tests::resources::source() / "killer_judge.py");
    runInfo();
}

BOOST_AUTO_TEST_CASE(kill_all)
{
    setJudge(dir::tests::resources::source() / "killer_judge.py");
    addSolution(3, "sleep", "5");
    addSolution(4, "sleep", "5");
    addSolution(5, "sleep", "5");
    runInfo();
}

BOOST_AUTO_TEST_CASE(echo)
{
    setJudge(dir::tests::resources::source() / "echo_judge.py");
    addSolutionCopy(3, dir::tests::resources::source() / "echo_solution.py");
    runInfo();
}

BOOST_AUTO_TEST_CASE(echo_ok)
{
    setJudge(dir::tests::resources::source() / "echo_ok_judge.py");
    addSolutionCopy(3, dir::tests::resources::source() / "echo_solution.py");
    runOK();
}

BOOST_AUTO_TEST_CASE(echo_loop)
{
    setJudge(dir::tests::resources::source() / "echo_loop_judge.py");
    addSolutionCopy(3, dir::tests::resources::source() / "echo_loop_solution.py");
    runOK();
}

BOOST_AUTO_TEST_CASE(echo_no_reply_exit)
{
    setJudge(dir::tests::resources::source() / "echo_loop_no_reply_judge.py");
    addSolutionCopy(3, dir::tests::resources::source() / "echo_loop_no_reply_solution.py");
    runOK();
}

BOOST_AUTO_TEST_CASE(echo_eof_after_terminate)
{
    setJudge(dir::tests::resources::source() / "echo_terminate_judge.py");
    addSolutionCopy(3, dir::tests::resources::source() / "echo_loop_no_reply_solution.py");
    runOK();
}

BOOST_AUTO_TEST_SUITE(resource_limits)

BOOST_AUTO_TEST_CASE(real_time_limit)
{
    setJudge(dir::tests::resources::source() / "sleep_rtl_judge.py");
    addSolutionCopy(3, dir::tests::resources::source() / "sleep_solution.py");
    runInfo();
}

BOOST_AUTO_TEST_SUITE_END() // resource_limits

BOOST_AUTO_TEST_SUITE_END()
