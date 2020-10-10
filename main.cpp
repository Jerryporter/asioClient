// asioClient.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#define BOOST_ASIO_DISABLE_STD_CHRONO
#define BOOST_CHRONO_HEADER_ONLY
#define BOOST_CHRONO_EXTENSIONS
#define BOOST_ASIO_ENABLE_HANDLER_TRACKING
#include <boost/chrono.hpp>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/system_timer.hpp>
#include <boost/asio/high_resolution_timer.hpp>
#include <boost/timer.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <vector>
#include "nlohmann/json.hpp"
using namespace boost::asio;
using namespace boost::chrono;
using namespace boost::system;
using namespace boost;
using json = nlohmann::json;
using  std::cin;
using std::cout;
using std::endl;
using std::vector;
using boost::shared_ptr;
seconds operator"" _s(unsigned long long n) {
    return seconds(n);
}
class shared_const_buffer
{
public:
    // Construct from a std::string.
    explicit shared_const_buffer(const std::string& data)
            : data_(new std::vector<char>(data.begin(), data.end())),
              buffer_(boost::asio::buffer(*data_))
    {
    }

    // Implement the ConstBufferSequence requirements.
    typedef boost::asio::const_buffer value_type;
    typedef const boost::asio::const_buffer* const_iterator;
    const boost::asio::const_buffer* begin() const { return &buffer_; }
    const boost::asio::const_buffer* end() const { return &buffer_ + 1; }

private:
    std::shared_ptr<std::vector<char>> data_;
    boost::asio::const_buffer buffer_;
};
class timerWithFunc
{
private:
    int m_count = 0;
    int m_count_max = 0;
    function<void()> m_f;
    steady_timer m_t;
public:
    template<typename F>
    timerWithFunc(io_service& io, int x, F func) :m_count_max(x), m_f(func), m_t(io, 5_s) {
        init();
    }
private:
    typedef timerWithFunc this_type;
    void init() {
        m_t.async_wait(boost::bind(&this_type::handler, this, boost::asio::placeholders::error));
    }

    void handler(const error_code&)
    {
        m_f();
        //m_t.expires_from_now(milliseconds(1));
        //m_t.async_wait(bind(&this_type::handler, this, boost::asio::placeholders::error));
    }
};

class client {
    typedef client this_type;
    typedef ip::tcp::endpoint endpoint_type;
    typedef ip::address address_type;
    typedef ip::tcp::socket socket_type;
    typedef shared_ptr<socket_type> sock_ptr;
    typedef vector<char> buffer_type;

private:
    io_service	m_io;
    buffer_type		m_buf;
    endpoint_type	m_ep;
    steady_timer m_tm;
    json mess;
public:
    client(json mess2) :
            m_buf(5, 0),
            m_ep(address_type::from_string("192.168.3.5"), 0x1234),
           // m_ep(address_type::from_string("192.168.31.96"), 8899),
            m_tm(m_io, 3_s)
            , mess(mess2)
    {

        start();
    }

    void run()
    {
        m_io.run();
    }

    void start()
    {
        sock_ptr sock(new socket_type(m_io));

        sock->connect(m_ep);
        send_mess(sock);

        m_tm.async_wait(
                [](const error_code& ec) {
                    //sock->send(buffer("hello?"));
                });
    }

    void send_mess(sock_ptr sock) {
        cout << mess.dump() << "	sendmess!" << endl;
        //int length = mess.dump().length();
        int length = mess.dump().length();
        sock->async_send(buffer((void*)&length, sizeof(length)),
                         bind(&this_type::send_handler, this, boost::asio::placeholders::error, sock));
    }
    //发送内容，同时设定倒计时
    void send_handler(const error_code& ec, sock_ptr sock)
    {
        shared_const_buffer buffer_1(mess.dump());
        //cout << mess.dump() << endl;
        sock->async_send(buffer_1,
                         [](const error_code& ec, std::size_t) {
                             if (ec)
                             {
                                 cout << "mess send error!" << ec.value() << "+" << ec.message() << endl;
                             }
                             cout << "mess send complete!" << endl;
                         });

    }

    void conn_handler(const error_code& ec, sock_ptr sock)
    {
        if (ec)
        {
            return;
        }
        cout << "recive from " << endl;
        //int length = mess.length();
        sock->async_write_some(buffer("hello"),
                               bind(&this_type::send_handler, this, boost::asio::placeholders::error, sock));

    }
};

int main()
{
    //json j = R"({"protocol": "keepalive"})"_json;
    //std::string s = j.dump();
    //string_ref ss(s);
    //cout << ss.data() << endl;
    //io_service io;
    //ip::tcp::socket sock(io);
    //ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 0x1234);
    //sock.connect(ep);

    //sock.async_send(buffer(j.dump()), [](const error_code& ,std::size_t) {cout << "send complete" << endl; });
    try {
        json j = R"({"protocol": "keepalive"})"_json;
        client c1(j);
        c1.run();
    }
    catch (const std::exception& e) {
        e.what();
    }

}