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
#include <boost/timer/timer.hpp>
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
using namespace boost;
using json = nlohmann::json;
using std::cin;
using std::cout;
using std::endl;
using std::vector;
using boost::shared_ptr;
using boost::asio::ip::udp;

boost::chrono::seconds operator "" _s(unsigned long long n) {
    return boost::chrono::seconds(n);
}

class shared_const_buffer {
public:
    // Construct from a std::string.
    explicit shared_const_buffer(const std::string &data)
            : data_(new std::vector<char>(data.begin(), data.end())),
              buffer_(boost::asio::buffer(*data_)) {
    }

    // Implement the ConstBufferSequence requirements.
    typedef boost::asio::const_buffer value_type;
    typedef const boost::asio::const_buffer *const_iterator;

    const boost::asio::const_buffer *begin() const { return &buffer_; }

    const boost::asio::const_buffer *end() const { return &buffer_ + 1; }

private:
    std::shared_ptr<std::vector<char>> data_;
    boost::asio::const_buffer buffer_;
};


std::string messFromGnu;

typedef ip::tcp::endpoint endpoint_type;
typedef ip::address address_type;
typedef ip::tcp::socket socket_type;
typedef shared_ptr<socket_type> sock_ptr;
typedef vector<char> buffer_type;
typedef std::function<void(const boost::system::error_code &)> timer_callback;

#include "mingw.thread.h"

void timerC(sock_ptr sock) {
    io_context io;
    steady_timer tt(io, 10_s);
    json j = R"({"protocol": "keepalive"})"_json;
    std::string s = j.dump();
    while (1) {
        cout << "in thread" << endl;
        tt.wait();
        std::size_t length2 = s.length();
        sock->send(buffer((void *) &length2, sizeof(length2)));
        shared_const_buffer buffer_1(s);
        sock->send(buffer_1);
        tt.expires_after(boost::chrono::seconds(10));
    }
}


int main() {
    boost::asio::io_context io_context;
    udp::socket sock2(io_context, udp::endpoint(ip::address::from_string("192.168.43.161"), 1666));
    sock_ptr sock(new socket_type(io_context));
    sock->connect(endpoint_type(address_type::from_string("192.168.43.162"), 8899));

    std::thread thread01(timerC, sock);
    thread01.detach();
    cout << "in main" << endl;

    while (1) {
        char data[1024];
        udp::endpoint sender_endpoint;
        sock2.receive_from(
                boost::asio::buffer(data, 1024), sender_endpoint);
        cout << data << endl;

        messFromGnu = data;

        std::size_t length2 = messFromGnu.length();
        //sock->async_send(buffer((void *) &length2, sizeof(length2)), [](const error_code &ec, std::size_t t) {});
        sock->send(buffer((void *) &length2, sizeof(length2)));
        shared_const_buffer buffer_1(messFromGnu);
        //sock->async_send(buffer_1, [](const error_code &ec, std::size_t t) {});
        sock->send(buffer_1);

    }
}