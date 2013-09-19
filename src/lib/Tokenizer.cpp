#include <yandex/contest/invoker/flowctl/game/Tokenizer.hpp>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    Tokenizer::~Tokenizer() {}

    Tokenizer::Result Tokenizer::operator()(const std::string &string)
    {
        return (*this)(string.data(), string.size());
    }

    Tokenizer::Result Tokenizer::operator()(const std::vector<char> &string)
    {
        return (*this)(&string[0], string.size());
    }

    const char *Tokenizer::what() const noexcept
    {
        return nullptr;
    }
}}}}}
