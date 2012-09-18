#include "yandex/contest/invoker/flowctl/game/BrokerTextInterface.hpp"

#include <utility>
#include <type_traits>

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

namespace yandex{namespace contest{namespace invoker{namespace flowctl{namespace game
{
    namespace
    {
        template <typename Iter>
        void escape(std::ostream &output, Iter begin, const Iter &end)
        {
            for (; begin != end; ++begin)
            {
                switch (*begin)
                {
                case ' ':
                    output << "\\ ";
                    break;
                case '\n':
                    output << "\\n";
                    break;
                case '\\':
                    output << "\\\\";
                    break;
                default:
                    output << *begin;
                }
            }
        }

        template <typename T>
        void escape(std::ostream &output, T &&obj)
        {
            escape(output, begin(obj), end(obj));
        }

        std::string unescape(std::istream &input, bool &end)
        {
            if (!input)
                BOOST_THROW_EXCEPTION(EndOfFileError());
            enum State
            {
                ESCAPE,
                ORDINAL
            };
            std::string buf;
            char c;
            State state = ORDINAL;
            while (input.get(c) && c != '\n' && c != ' ')
            {
                switch (state)
                {
                case ESCAPE:
                    switch (c)
                    {
                    case '\\':
                    case ' ':
                        buf.push_back(c);
                        break;
                    case 'n':
                        buf.push_back('\n');
                        break;
                    default:
                        BOOST_ASSERT(false);
                    }
                    state = ORDINAL;
                    break;
                case ORDINAL:
                    switch (c)
                    {
                    case '\\':
                        state = ESCAPE;
                        break;
                    default:
                        buf.push_back(c);
                    }
                    break;
                }
            }
            BOOST_ASSERT(state != ESCAPE);
            end = !input || c == '\n';
            if (!end)
                BOOST_ASSERT(c == ' ');
            return buf;
        }

        namespace traits
        {
            template <typename T>
            struct Streamable:
                std::integral_constant<bool,
                    std::is_enum<T>::value || std::is_integral<T>::value> {};

            template <>
            struct Streamable<std::string>: std::integral_constant<bool, true> {};
        };

        class OutputArchive
        {
        public:
            explicit OutputArchive(std::ostream &output): output_(output) {}

            template <typename T>
            OutputArchive &operator&(const T &obj)
            {
                return (*this) << obj;
            }

            template <typename T>
            OutputArchive &operator<<(const T &obj)
            {
                save(obj);
                return *this;
            }

            template <typename T>
            OutputArchive &operator<<(const boost::serialization::nvp<T> &obj)
            {
                return (*this) << obj.value();
            }

            ~OutputArchive()
            {
                output_ << std::endl;
            }

        private:
            template <typename T>
            typename std::enable_if<!traits::Streamable<T>::value, void>::type
            save(const T &obj)
            {
                // FIXME use boost::serialization::access
                const_cast<T &>(obj).serialize(*this, 0);
            }

            template <typename T>
            typename std::enable_if<traits::Streamable<T>::value, void>::type
            save(const T &obj)
            {
                print(obj);
            }

            template <typename T>
            void save(const std::vector<T> &v)
            {
                for (const T &obj: v)
                    save(obj);
            }

        private:
            template <typename T>
            void print(const T &obj)
            {
                if (!first)
                    output_ << ' ';
                first = false;
                escape(output_, boost::lexical_cast<std::string>(obj));
            }

        private:
            std::ostream &output_;
            bool first = true;
        };

        class InputArchive
        {
        public:
            explicit InputArchive(std::istream &input): input_(input) {}

            template <typename T>
            InputArchive &operator&(T &obj)
            {
                return (*this) >> obj;
            }

            template <typename T>
            InputArchive &operator>>(T &obj)
            {
                BOOST_ASSERT(!end_);
                load(obj);
                return *this;
            }

            template <typename T>
            InputArchive &operator>>(const boost::serialization::nvp<T> &obj)
            {
                return (*this) >> obj.value();
            }

        private:
            template <typename T>
            typename std::enable_if<!traits::Streamable<T>::value, void>::type
            load(T &obj)
            {
                // FIXME use boost::serialization::access
                obj.serialize(*this, 0);
            }

            template <typename T>
            typename std::enable_if<traits::Streamable<T>::value, void>::type
            load(T &obj)
            {
                obj = boost::lexical_cast<T>(unescape(input_, end_));
            }

            template <typename T>
            void load(std::vector<T> &v)
            {
                v.clear();
                while (!end_)
                {
                    T obj;
                    load(obj);
                    v.push_back(std::move(obj));
                }
            }

        private:
            std::istream &input_;
            bool end_ = false;
        };
    }

    BrokerTextInterface::BrokerTextInterface(std::istream &input, std::ostream &output):
        input_(input), output_(output) {}

    void BrokerTextInterface::send(const Command &command)
    {
        OutputArchive oa(output_);
        oa << command;
    }

    void BrokerTextInterface::send(const Result &result)
    {
        OutputArchive oa(output_);
        oa << result;
    }

    BrokerTextInterface::Command BrokerTextInterface::recvCommand()
    {
        Command command;
        InputArchive ia(input_);
        ia >> command;
        return command;
    }

    BrokerTextInterface::Result BrokerTextInterface::recvResult()
    {
        Result result;
        InputArchive ia(input_);
        ia >> result;
        return result;
    }
}}}}}
