#include "yandex/contest/invoker/flowctl/game/KillerSTDInterface.hpp"

#include "yandex/contest/SystemError.hpp"

#include <iostream>
#include <type_traits>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    namespace
    {
        template <typename T>
        void sendBytes(const T &obj)
        {
            constexpr std::streamsize size = sizeof(T);
            static_assert(std::is_pod<T>::value, "Should be pod.");
            std::cout.write(reinterpret_cast<const char *>(&obj), size);
            std::cout.flush();
            if (!std::cout)
                BOOST_THROW_EXCEPTION(SystemError("write"));
        }

        template <typename T>
        T recvBytes()
        {
            constexpr std::streamsize size = sizeof(T);
            static_assert(std::is_pod<T>::value, "Should be pod.");
            T obj;
            std::cin.read(reinterpret_cast<char *>(&obj), size);
            if (std::cin.gcount() < size)
                BOOST_THROW_EXCEPTION(EndOfFileError());
            return obj;
        }
    }

    void KillerSTDInterface::send(const Command &command)
    {
        return sendBytes(command);
    }

    void KillerSTDInterface::send(const Status &status)
    {
        return sendBytes(status);
    }

    KillerSTDInterface::Command KillerSTDInterface::recvCommand()
    {
        return recvBytes<Command>();
    }

    KillerSTDInterface::Status KillerSTDInterface::recvStatus()
    {
        return recvBytes<Status>();
    }
}}}}}
