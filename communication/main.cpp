#include <iostream>

#include <boost/asio.hpp>

using boost::asio::ip::udp;

class ClientSet {
public:
  udp::endpoint m_clientEndpoint;
  std::string m_nickname;
  bool m_online;
  ClientSet(const std::string& name, udp::endpoint& clientEndpoint)
  : m_clientEndpoint(clientEndpoint), m_nickname(name), m_online(true) {}
};

class WaServer {

  boost::asio::io_service& m_io_service;
  udp::socket m_socket;
  std::array<uint8_t, 255> m_buffer;
  udp::endpoint m_sender_endpoint;
  std::vector<ClientSet> clientList;

  void set_async_receive() {
    m_socket.async_receive_from(
            boost::asio::buffer(m_buffer), m_sender_endpoint,
            [this](const boost::system::error_code& error, size_t bytes_recvd){receiveHandler(error, bytes_recvd); });
  }

  std::string getNickname(const udp::endpoint& sender) {
    std::string nickname("unknown");
    for (auto client : clientList)
      if (client.m_clientEndpoint == sender)
        nickname = client.m_nickname;
    return nickname;
  }

  udp::endpoint getEndpoint(const std::string& nickname) {
    udp::endpoint ep;
    for (auto client : clientList)
      if (client.m_nickname == nickname)
        ep = client.m_clientEndpoint;
    return ep;
  }

public:
  WaServer(boost::asio::io_service& service, uint16_t port = 12001)
  : m_io_service(service), m_socket(m_io_service, udp::endpoint(udp::v4() /* any UDP */, port)) {

    // now, we need to listen to any IP, and react on incomming data
    set_async_receive();
  }

  void receiveHandler(const boost::system::error_code& error,
                      size_t bytes_recvd) {

    if (!error && bytes_recvd > 0) {

      // simplify with strings
      std::string data((char*)&m_buffer[0],bytes_recvd);

      if (data.substr(0,8) == "register")
        do_register(data.substr(9));

      if (data.substr(0,9) == "broadcast")
        do_broadcast(data.substr(10));

      if (data.substr(0,4) == "send")
        do_send(data.substr(5));
    }
    else
      std::cout << "error\n";

    set_async_receive();
  }


  void do_register(const std::string& nickname) {
    std::cout << "new client registered <" << nickname << ">\n";

    // informing all other clients about the new one and send available clients to new one
    std::string ownName("newone "+nickname);
    for (auto client : clientList) {
      std::string msg("newone "+client.m_nickname);
      m_socket.send_to(boost::asio::buffer(msg), m_sender_endpoint);
      m_socket.send_to(boost::asio::buffer(ownName), client.m_clientEndpoint);
    }

    // keep client in our databas
    clientList.push_back(ClientSet(nickname, m_sender_endpoint));
  }

  void do_broadcast(const std::string& message) {
    std::cout << "new broadcast message <" << message << ">\n";

    std::string msg("send "+getNickname(m_sender_endpoint)+" "+message);

    std::for_each(std::begin(clientList),std::end(clientList),
                  [&](ClientSet& cs)
                  { if (m_sender_endpoint != cs.m_clientEndpoint)
                    m_socket.send_to(boost::asio::buffer(msg), cs.m_clientEndpoint);
                  });
  }

  void do_send(const std::string& remainder) {
    std::string::size_type nickEnd = remainder.find_first_of(' ');
    std::string toNickName = remainder.substr(0,nickEnd);
    std::string message = "send " + getNickname(m_sender_endpoint) + " " + remainder.substr(nickEnd+1);

    udp::endpoint ep = getEndpoint(toNickName);

    if (ep.port() == 0)
      std::cout << "no client found for nick <"<<toNickName<<">\n";
    else
      m_socket.send_to(boost::asio::buffer(message), ep);

  }

};

#ifndef ALL_MAIN
int main() {

  boost::asio::io_service io_service;

  WaServer server(io_service);

  io_service.run();

  return 0;
}
#endif
