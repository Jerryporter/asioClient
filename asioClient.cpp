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
	std::string jj;
	string_ref ch;
public:
	client(json mess) :
		m_buf(5, 0),
		m_ep(address_type::from_string("127.0.0.1"), 0x1234),
		m_tm(m_io, 3_s),
		mess(mess),
		jj(mess.dump()),
		ch(jj)
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
		cout << "recive from " << endl;
		//int length = mess.dump().length();
		//std::string set = "lkasjdglka";
		sock->async_send(buffer(lexical_cast<std::string>(10)),
			bind(&this_type::send_handler, this, boost::asio::placeholders::error, sock));
	}
	//发送内容，同时设定倒计时
	void send_handler(const error_code& ec, sock_ptr sock)
	{
		//cout << mess.dump() << endl;
		//sock->async_send(buffer(ch.data(), ch.size()),
		sock->async_send(buffer("sdfhfh"),
			[&](const error_code& ec, std::size_t) {
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
	std::string s = "lsdjglkj";
	string_ref ss(s);
	cout << ss.data() << endl;
	try
	{
		json j = R"({"protocol": "keepalive"})"_json;
		client c1(j);
		c1.run();
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
	}

}