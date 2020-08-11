#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <string>
#include <iostream>

const unsigned int PORT = 11111;

using WSStream = boost::beast::websocket::stream<boost::asio::ip::tcp::socket>;

boost::asio::io_context ioc;
WSStream client_stream (ioc);
WSStream server_stream (ioc);
boost::asio::ip::tcp::resolver resolver (ioc);
boost::asio::ip::tcp::resolver::results_type resolved_address;
boost::asio::ip::tcp::acceptor acceptor (ioc);
boost::asio::deadline_timer timer (ioc);
boost::asio::streambuf recv_data;

// server process
void server_connect();
void on_accept(const boost::system::error_code&);
void on_ws_accept(const boost::system::error_code&);
void on_read(const boost::system::error_code&, size_t);

//client process
void on_resolve(const boost::system::error_code&,boost::asio::ip::tcp::resolver::results_type);
void client_connect();
void on_connect(const boost::system::error_code&, const boost::asio::ip::tcp::endpoint&);
void on_handshake(const boost::system::error_code&);
void on_pause(const boost::system::error_code&);


void server_connect()
{
	acceptor.async_accept(server_stream.next_layer(),on_accept);
}

void on_accept(const boost::system::error_code& ec)
{
	if (ec)
	{
		std::cout << "tcp accept error" << std::endl;
		ioc.stop();
		return;
	}
	std::cout << "Will it crash?... " << std::endl;
	server_stream.async_accept(on_ws_accept);
	std::cout << "not yet!" << std::endl;
}

void on_ws_accept(const boost::system::error_code& ec)
{
	if (ec)
	{
		std::cout << "websocket accept error" << std::endl;
		ioc.stop();
		return;
	}
	std::cout << "Connection established" << std::endl;
	server_stream.async_read(recv_data,on_read);
} 

void on_read(const boost::system::error_code& ec, size_t len)
{
	if (ec)
	{
		std::cout << "Connection finished: " << ec.message() << std::endl;
		server_stream.next_layer().close();
		server_connect();
		return;
	}
	std::cout << "something was read. This should not happen" << std::endl;
	ioc.stop();
}

void on_resolve(const boost::system::error_code& ec,boost::asio::ip::tcp::resolver::results_type results)
{
	if (ec)
	{
		std::cout << "resolver error" << std::endl;
		ioc.stop();
		return;
	}
	resolved_address = results;
	client_connect();
}

void client_connect()
{
	boost::asio::async_connect(client_stream.next_layer(),resolved_address,on_connect);
}

void on_connect(const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint&)
{
	if (ec)
	{
		std::cout << "connection error" << std::endl;
		ioc.stop();
		return;
	}
	client_stream.async_handshake("localhost","/",on_handshake);
}

void on_handshake(const boost::system::error_code& ec)
{
	if (ec)
	{
		std::cout << "handshake error" << std::endl;
		ioc.stop();
		return;
	}
	timer.expires_from_now(boost::posix_time::milliseconds(1000));
	timer.async_wait(on_pause);
}

void on_pause(const boost::system::error_code&)
{
	client_stream.next_layer().close();

	// to avoid client errors, rebuild client stream
	client_stream.~WSStream();
	new (&client_stream) WSStream(ioc);

	client_connect();
}


int main()
{
	// acceptor setup
	boost::asio::ip::tcp::endpoint ep (boost::asio::ip::tcp::v4(), PORT);
    acceptor.open(ep.protocol());
    acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor.bind(ep);
    acceptor.listen();

    // start client / server
    server_connect();
    resolver.async_resolve("localhost",std::to_string(PORT),on_resolve);
    ioc.run();
    
    std::cout << "Program finished" << std::endl;
}