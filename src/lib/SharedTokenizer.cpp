#include "yandex/contest/invoker/flowctl/game/SharedTokenizer.hpp"

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    constexpr const char *SharedTokenizerFactory::newTokenizerName;
    constexpr const char *SharedTokenizerFactory::deleteTokenizerName;
    constexpr const char *SharedTokenizerFactory::parseName;
    constexpr const char *SharedTokenizerFactory::whatName;

    class SharedTokenizerFactory::Instance: public Tokenizer
    {
    public:
        Instance(const newTokenizerFunction newTokenizer,
                 const deleteTokenizerFunction deleteTokenizer,
                 const parseFunction parse,
                 const whatFunction what,
                 const char *const argument):
            newTokenizer_(newTokenizer),
            deleteTokenizer_(deleteTokenizer),
            parse_(parse),
            what_(what),
            instance_(newTokenizer_(argument))
        {
            if (!instance_)
                throw std::bad_alloc();
        }

        virtual ~Instance()
        {
            BOOST_ASSERT(instance_);
            deleteTokenizer_(instance_);
        }

        virtual Result operator()(const char *data, std::size_t size) override
        {
            const int result = parse_(instance_, data, &size);
            if (result < 0)
                return Result{Status::FAIL, size};
            else if (result == 0)
                return Result{Status::CONTINUE, size};
            else
                return Result{Status::ACCEPTED, size};
        }

        virtual const char *what() const noexcept override
        {
            if (what_)
                return what_(instance_);
            else
                return Tokenizer::what();
        }

    private:
        newTokenizerFunction newTokenizer_;
        deleteTokenizerFunction deleteTokenizer_;
        parseFunction parse_;
        whatFunction what_;
        void *const instance_ = nullptr;
    };

    SharedTokenizerFactory::SharedTokenizerFactory(system::unistd::DynamicLibrary &dl):
        newTokenizer_(reinterpret_cast<newTokenizerFunction>(dl.symbol(newTokenizerName))),
        deleteTokenizer_(reinterpret_cast<deleteTokenizerFunction>(dl.symbol(deleteTokenizerName))),
        parse_(reinterpret_cast<parseFunction>(dl.symbol(parseName))),
        what_(reinterpret_cast<whatFunction>(dl.symbol(whatName)))
    {
        BOOST_ASSERT(newTokenizer_);
        BOOST_ASSERT(deleteTokenizer_);
        BOOST_ASSERT(parse_);
    }

    std::unique_ptr<Tokenizer> SharedTokenizerFactory::instance(const std::string &argument)
    {
        std::unique_ptr<Tokenizer> ptr(new Instance(
            newTokenizer_, deleteTokenizer_, parse_, what_, argument.c_str()));
        return ptr;
    }
}}}}}
