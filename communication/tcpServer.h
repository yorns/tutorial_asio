//
// Created by embed on 29.01.17.
//

#ifndef WA_CLONE_TCPSERVER_H
#define WA_CLONE_TCPSERVER_H

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <array>
#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class ClientSession : public std::enable_shared_from_this<ClientSession> {
public:
  tcp::socket m_socket;
  std::string m_nickname;
  static constexpr uint32_t maxLength = 1024;

  ClientSession(tcp::socket socket)
          : m_socket(std::move(socket)), m_nickname("dave") {}

  void start() { do_read(); }

  void do_read()
  {
    auto self(shared_from_this());
    m_socket.async_read_some(boost::asio::buffer(m_input_buffer),
                            [this, self](boost::system::error_code ec, std::size_t length) {
                              if (!ec) {
                                std::string msg(&m_input_buffer[0], length);
                                  do_write(m_nickname+" you said: "+msg);
                              }
                            });
  }

  void do_write(const std::string& msg)
  {
    auto self(shared_from_this());
    auto ret_msg = std::make_shared<std::string>(msg);
    boost::asio::async_write(m_socket, boost::asio::buffer(*ret_msg),
                             [this, self, ret_msg](boost::system::error_code ec, std::size_t /*length*/) {
                               if (!ec) do_read();
                             });
  }

  std::array<char, maxLength> m_input_buffer;
};

class server
{
public:
  server(boost::asio::io_service& io_service, short port)
          : m_acceptor(io_service, tcp::endpoint(tcp::v4(), port)),
            m_socket(io_service)
  {
    do_accept();
  }

private:
  void do_accept()
  {
    m_acceptor.async_accept(m_socket,
                           [this](boost::system::error_code ec) {
                             if (!ec)
                               std::make_shared<ClientSession>(std::move(m_socket))->start();
                            do_accept();
                           });
  }

  tcp::acceptor m_acceptor;
  tcp::socket m_socket;
};

#ifndef ALL_MAIN
int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: " << argv[0] << " <port>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    server s(io_service, std::atoi(argv[1]));

    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
#endif // ALL_MAIN
#endif //WA_CLONE_TCPSERVER_H
