// asioClient.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#define BOOST_ASIO_DISABLE_STD_CHRONO
#define BOOST_CHRONO_HEADER_ONLY
#define BOOST_CHRONO_EXTENSIONS
#define BOOST_ASIO_ENABLE_HANDLER_TRACKING
#define BOOST_THREAD_VERSION 5
#include <boost/thread.hpp>
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
using namespace boost::system;

using namespace boost::asio;
using namespace boost;
using json = nlohmann::json;
using std::cin;
using std::cout;
using std::endl;
using std::vector;
using boost::shared_ptr;
using boost::asio::ip::udp;

boost::chrono::seconds operator "" _s(unsigned long long n){
	return boost::chrono::seconds(n);
}

class shared_const_buffer{
public:
	// Construct from a std::string.
	explicit shared_const_buffer(const std::string& data)
		: data_(new std::vector<char>(data.begin(), data.end())),
		buffer_(boost::asio::buffer(*data_)){
	}

	// Implement the ConstBufferSequence requirements.
	typedef boost::asio::const_buffer value_type;
	typedef const boost::asio::const_buffer* const_iterator;

	const boost::asio::const_buffer* begin() const{
		return &buffer_;
	}

	const boost::asio::const_buffer* end() const{
		return &buffer_ + 1;
	}

private:
	std::shared_ptr<std::vector<char>> data_;
	boost::asio::const_buffer buffer_;
};

typedef ip::tcp::socket socket_type;
typedef shared_ptr<socket_type> sock_ptr;
typedef vector<char> buffer_type;

#define KEEP_LIVE_INTERVAL 60_s
#define UDP_ENDPOINT ip::udp::endpoint(ip::address::from_string("127.0.0.1"),1666)
#define TCP_ENDPOINT_LOOP ip::tcp::endpoint(ip::address::from_string("127.0.0.1"),0x1234)
//#define TCP_ENDPOINT_REMOTE tcp::endpoint(ip::address::from_string("192.168.43.161")

//计时器线程，保证保活报文的发送
void timerC(sock_ptr sock){
	io_context io;
	steady_timer tt(io, KEEP_LIVE_INTERVAL);
	json keep_alive_mess_json = R"({"protocol": "keepalive"})"_json;
	std::string s = keep_alive_mess_json.dump();
	while (1){
		cout << "in thread" << endl;
		tt.wait();
		std::size_t str_length = s.length();
		sock->send(buffer((void*)&str_length, sizeof(str_length)));
		shared_const_buffer buffer_1(s);
		sock->send(buffer_1);
		tt.expires_after(boost::chrono::seconds(KEEP_LIVE_INTERVAL));
	}
}

int main(){
	//set up io_context and udp,tcp socket
	boost::asio::io_context io_context;
	udp::socket sock_gnu(io_context, UDP_ENDPOINT);
	sock_ptr sock(new socket_type(io_context));
	//connect tcp server and define buffer
	sock->connect(TCP_ENDPOINT_LOOP);
	cout << sock_gnu.available() << endl;

	//set up keep live mess send thread

	//boost::thread thread_keeptcplive(timerC, sock);
	udp::endpoint sender_endpoint;
	cout << "in main" << endl;
	char data[3096];
	std::string mess_str;
	while (1){
		sock_gnu.receive_from(boost::asio::buffer(data), sender_endpoint);
		cout << &data[0] << endl;
		mess_str = data;
		if (true){
			std::size_t length_data = mess_str.length();
			//sock->async_send(buffer((void*)&length_data, sizeof(length_data)), [](const error_code& ec, std::size_t t){});
			sock->send(buffer((void*)&length_data, sizeof(length_data)));

			shared_const_buffer buffer_1(data);
			//sock->async_send(buffer_1, [](const error_code& ec, std::size_t t){});
			sock->send(buffer_1);
		}
		mess_str.clear();
	}
}
