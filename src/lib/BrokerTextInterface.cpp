#include "yandex/contest/invoker/flowctl/game/BrokerTextInterface.hpp"

#include "yandex/contest/SystemError.hpp"
#include "yandex/contest/TypeInfo.hpp"

#include "yandex/contest/detail/LogHelper.hpp"

#include <utility>
#include <type_traits>

#include <cstring>

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
            if (end)
                BOOST_THROW_EXCEPTION(EndOfFileError());
            enum State
            {
                INIT,
                ORDINAL,
                ESCAPE
            };
            std::string buf;
            char c;
            State state = INIT;
            while (input.get(c) && c != '\n' && !(state != ESCAPE && c == ' '))
            {
                switch (state)
                {
                case INIT:
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
                        {
                            char msg[] = "Unknown escape sequence \"\\_\".";
                            *std::strrchr(msg, '_') = c;
                            BOOST_ASSERT_MSG(false, msg);
                        }
                    }
                    state = ORDINAL;
                    break;
                }
            }
            if (!input && state == INIT)
                BOOST_THROW_EXCEPTION(EndOfFileError());
            if (input.bad())
                BOOST_THROW_EXCEPTION(SystemError("read"));
            BOOST_ASSERT(state != ESCAPE);
            end = !input || c == '\n';
            BOOST_ASSERT(end || c == ' ');
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
                BOOST_ASSERT(!last_);
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
                    (*this) << obj;
                last_ = true;
            }

        private:
            template <typename T>
            void print(const T &obj)
            {
                if (!first_)
                    output_ << ' ';
                first_ = false;
                escape(output_, boost::lexical_cast<std::string>(obj));
            }

        private:
            std::ostream &output_;
            bool first_ = true;
            bool last_ = false;
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
                STREAM_TRACE << "Trying to read value type = \"" << typeinfo::name(obj) << "\".";
                if (end_)
                    BOOST_THROW_EXCEPTION(EndOfFileError());
                BOOST_ASSERT(!last_);
                load(obj);
                STREAM_TRACE << "Object of type = \"" << typeinfo::name(obj) << "\" was read," <<
                                " end = " << end_ << ".";
                return *this;
            }

            /// \note end_ is treated in special way here
            template <typename T>
            InputArchive &operator>>(std::vector<T> &obj)
            {
                STREAM_TRACE << "Trying to read value of vector type = \"" <<
                                typeinfo::name(obj) << "\".";
                BOOST_ASSERT(!last_);
                load(obj);
                STREAM_TRACE << "Object of vector type = \"" << typeinfo::name(obj) <<
                                "\" was read, end = " << end_ << ".";
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
                    (*this) >> obj;
                    v.push_back(std::move(obj));
                }
                last_ = true;
            }

        private:
            std::istream &input_;
            bool end_ = false;
            bool last_ = false;
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
