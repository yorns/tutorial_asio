#include <iostream>
#include <memory>
#include <vector>
#include <functional>
#include <string>
#include <boost/asio.hpp>

class Machine {

  boost::asio::io_service& m_service;
  boost::asio::deadline_timer m_timer;
  std::string m_name;
  uint32_t m_duration;

  std::function<void(const std::string&)> insert;

  void machine_finished(){
    std::cout << m_name <<"\t finished\n";
    insert(m_name);
  }

public:

  Machine(boost::asio::io_service& service, const std::string& name, uint32_t duration)
          : m_service(service), m_timer(service), m_name(name), m_duration(duration)
  {
  }

  void machine_start() {
    std::cout << m_name << "\t started\n";
    m_timer.expires_from_now(boost::posix_time::seconds(m_duration));
    m_timer.async_wait([this](boost::system::error_code){ machine_finished(); });
  }

  void setInserter(std::function<void(const std::string&)> func) { insert = func; }

};

class Mix {

  boost::asio::io_service& m_service;
  boost::asio::deadline_timer m_timer;

  std::vector<std::function<void()>> m_inputStartList;
  uint32_t pott;

  bool m_running = false;
  bool m_stop = false;

  void machine_start() {
    for (auto& input_start : m_inputStartList) {
      input_start();
    }
  }

  void test_for_start() {
    if ( pott == 7 && !m_running ) {
      std::cout << " --- Mixing started\n";
      m_running = true;
      m_timer.expires_from_now(boost::posix_time::seconds(5));
      m_timer.async_wait([this](boost::system::error_code){ machine_finished(); });
      pott = 0;
      if (!m_stop)
        machine_start();
    }
  }


  void machine_finished() {
    std::cout << " --- Mixing ended\n";
    m_running = false;
    test_for_start();
  }

public:
  Mix(boost::asio::io_service& service) :
          m_service(service), m_timer(service), pott(0) {
  }

  void insert(const std::string& input)    {
    if (input == "egg")
      pott |= 1;
    if (input == "flor")
      pott |= 2;
    if (input == "butter")
      pott |= 4;
    test_for_start();
  }

  void add_start(std::function<void()> start) {
    m_inputStartList.push_back(start);
  }

  void set_stop() {
    std::cout << "## STOP ##\n";
    m_stop = true;
  }
};

int main() {

  boost::asio::io_service service;
  Machine mehl(service, "flor", 3);
  Machine eier(service, "egg", 4);
  Machine butter(service, "butter", 1);

  Mix mixi(service);

  mixi.add_start([&mehl](){ mehl.machine_start();});
  mixi.add_start([&eier](){ eier.machine_start();});
  mixi.add_start([&butter](){ butter.machine_start();});

  mehl.setInserter([&mixi](const std::string& name){ mixi.insert(name);});
  eier.setInserter([&mixi](const std::string& name){ mixi.insert(name);});
  butter.setInserter([&mixi](const std::string& name){ mixi.insert(name);});

  mehl.machine_start();
  eier.machine_start();
  butter.machine_start();

  boost::asio::deadline_timer timer(service);
  timer.expires_from_now(boost::posix_time::seconds(60));
  timer.async_wait([&mixi](boost::system::error_code){ mixi.set_stop(); });

  service.run();

}