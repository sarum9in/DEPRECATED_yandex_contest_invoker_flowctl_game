#include "yandex/contest/invoker/flowctl/game/KillerStreamInterface.hpp"

#include "yandex/contest/SystemError.hpp"

#include <iostream>
#include <type_traits>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    namespace
    {
        template <typename T>
        void sendBytes(std::ostream &out, const T &obj)
        {
            constexpr std::streamsize size = sizeof(T);
            static_assert(std::is_pod<T>::value, "Should be pod.");
            out.write(reinterpret_cast<const char *>(&obj), size);
            out.flush();
            if (!out)
                BOOST_THROW_EXCEPTION(SystemError("write"));
        }

        template <typename T>
        T recvBytes(std::istream &in)
        {
            constexpr std::streamsize size = sizeof(T);
            static_assert(std::is_pod<T>::value, "Should be pod.");
            T obj;
            in.read(reinterpret_cast<char *>(&obj), size);
            if (in.gcount() < size)
                BOOST_THROW_EXCEPTION(EndOfFileError());
            return obj;
        }
    }

    KillerStreamInterface::KillerStreamInterface(std::istream &input, std::ostream &output):
        input_(input), output_(output) {}

    void KillerStreamInterface::send(const Command &command)
    {
        return sendBytes(output_, command);
    }

    void KillerStreamInterface::send(const Status &status)
    {
        return sendBytes(output_, status);
    }

    KillerStreamInterface::Command KillerStreamInterface::recvCommand()
    {
        return recvBytes<Command>(input_);
    }

    KillerStreamInterface::Status KillerStreamInterface::recvStatus()
    {
        return recvBytes<Status>(input_);
    }
}}}}}
