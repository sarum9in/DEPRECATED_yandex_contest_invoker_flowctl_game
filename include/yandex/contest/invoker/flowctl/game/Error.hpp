#pragma once

#include "yandex/contest/invoker/Error.hpp"

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    struct Error: virtual invoker::Error {};

    struct EndOfFileError: virtual Error {};
}}}}}
