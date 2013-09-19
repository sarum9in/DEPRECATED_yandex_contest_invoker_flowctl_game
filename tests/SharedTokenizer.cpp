#define BOOST_TEST_MODULE SharedTokenizer
#include <boost/test/unit_test.hpp>

#include <yandex/contest/invoker/flowctl/game/SharedTokenizer.hpp>

#include <dlfcn.h>

namespace ya = yandex::contest::invoker::flowctl::game;
namespace unistd = yandex::contest::system::unistd;

struct SharedTokenizerFixture
{
    SharedTokenizerFixture():
        split("yandex_contest_invoker_flowctl_game_tokenizer_split", RTLD_NOW),
        splitChar("yandex_contest_invoker_flowctl_game_tokenizer_split_char", RTLD_NOW),
        splitFactory(split),
        splitCharFactory(splitChar)
    {
    }

    void check(const std::string &msg,
               const ya::Tokenizer::Status status,
               const std::size_t accepted=0)
    {
        BOOST_REQUIRE(tok);
        const ya::Tokenizer::Result result = (*tok)(msg);
        BOOST_CHECK_EQUAL(result.status, status);
        if (status == ya::Tokenizer::Status::ACCEPTED)
            BOOST_CHECK_EQUAL(result.accepted, accepted);
    }

    void checkContinue(const std::string &msg)
    {
        check(msg, ya::Tokenizer::Status::CONTINUE);
    }

    void checkAccepted(const std::string &msg, const std::size_t size)
    {
        check(msg, ya::Tokenizer::Status::ACCEPTED, size);
    }

    unistd::DynamicLibrary split, splitChar;
    ya::SharedTokenizerFactory splitFactory, splitCharFactory;
    std::unique_ptr<ya::Tokenizer> tok;
};

BOOST_FIXTURE_TEST_SUITE(SharedTokenizer, SharedTokenizerFixture)

BOOST_AUTO_TEST_CASE(split_char)
{
    tok = splitFactory.instance("\n");
    checkContinue("123");
    checkAccepted("12\n45", 3);
}

BOOST_AUTO_TEST_SUITE(split)

BOOST_AUTO_TEST_CASE(simple)
{
    tok = splitFactory.instance("\n\n");
    BOOST_TEST_CHECKPOINT("1");
    checkContinue("123");
    BOOST_TEST_CHECKPOINT("2");
    checkContinue("123");
    BOOST_TEST_CHECKPOINT("3");
    checkAccepted("123\n\n", 5);
}

BOOST_AUTO_TEST_CASE(parted)
{
    tok = splitFactory.instance("\n\n");
    checkContinue("123\n");
    checkAccepted("\n23", 1);
}

BOOST_AUTO_TEST_CASE(overlapping0)
{
    tok = splitFactory.instance("121");
    checkAccepted("1121", 4);
}

#warning "See split tokenizer source, should be fixed in the future."
BOOST_AUTO_TEST_CASE_EXPECTED_FAILURES(overlapping2, 1)
BOOST_AUTO_TEST_CASE(overlapping2)
{
    tok = splitFactory.instance("axbayc");
    checkAccepted("axbaxbayc", 9);
}

BOOST_AUTO_TEST_SUITE_END() // split

BOOST_AUTO_TEST_SUITE_END() // SharedTokenizer
