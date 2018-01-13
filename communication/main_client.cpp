#include <iostream>
#include <chrono>
#include <thread>
#include <boost/asio.hpp>
#include "CommandLine.h"

using boost::asio::ip::udp;

typedef std::function<void(const std::string&)> OutputFunc;
class WaClient {

  boost::asio::io_service& m_service;
  udp::socket m_socket;
  udp::endpoint m_server_endpoint;
  std::array<char, 255> m_inBuffer;
  boost::asio::deadline_timer m_timer;
  OutputFunc m_outputFunc;
  bool m_stopped;
  udp::endpoint ep;

  void set_async_receive() {
    m_socket.async_receive_from(
            boost::asio::buffer(m_inBuffer), ep,
            std::bind(&WaClient::receiveHandler, this, std::placeholders::_1, std::placeholders::_2));
  }

public:
  WaClient(const std::string& name, boost::asio::io_service& service, const std::string& serverIp, uint16_t port) :
    m_service(service), m_socket(m_service, udp::endpoint(udp::v4(), 0 /* take random port */)),
    m_server_endpoint(boost::asio::ip::address::from_string(serverIp), port),
    m_timer(service, boost::posix_time::seconds(5)),
    m_stopped(false)
  {
    send("register "+ name);
    set_async_receive();
  }

  ~WaClient() { std::cout << "WaClient destructor\n"; }

  void receiveHandler(const boost::system::error_code& error,
                      size_t bytes_recvd)
  { if (m_stopped)
      return;
    if (!error && bytes_recvd>0) {
      std::string receivedData(&m_inBuffer[0], bytes_recvd);
      if (receivedData.substr(0, 6) == "newOne" && m_outputFunc)
        m_outputFunc( "new user <" + receivedData.substr(7) + ">");

      if (receivedData.substr(0, 4) == "send" && m_outputFunc) {
        m_outputFunc( "message received " + receivedData.substr(5) );
      }
    }
      set_async_receive();
  }

  void send(const std::string& data) {
    if (!m_stopped)
      m_socket.send_to(boost::asio::buffer(data), m_server_endpoint);
  }

  void setOutput(OutputFunc outputFunc) { m_outputFunc = outputFunc; }

  void stop()
  { m_stopped = true; m_socket.close(); }
};

int main(int argc, char* argv[]) {

  boost::asio::io_service io_service;

  WaClient client(argv[1], io_service, "127.0.0.1", 12001);
  CommandLine cmdLine(io_service);
  KeyHit keyHit;

  keyHit.setKeyReceiver(std::bind(&CommandLine::keyInputExternal, &cmdLine, std::placeholders::_1));
  cmdLine.setStop([&keyHit, &client](){keyHit.stop(); client.stop(); });
  cmdLine.setSend(std::bind(&WaClient::send, &client, std::placeholders::_1));
  client.setOutput(std::bind(&CommandLine::output, &cmdLine, std::placeholders::_1));

  cmdLine.output("starting");

  io_service.run();

  return 0;
}
