#include "main_mthread.h"
#include <thread>
#include <iostream>
#include <boost/asio.hpp>

void handler(const boost::system::error_code& error)
{
  if (!error) std::cout << "expire handler called\n";
  static int counter(0);
  std::cout << "-------------------------------------------------\n";
  std::cout << "call happens for "<< ++counter <<"th time\n";
  std::cout << "-------------------------------------------------\n";
}

static std::vector<uint32_t> dataList;

uint32_t storeInfo(uint32_t value)
{
  dataList.push_back(value);
  return dataList.size();
}


void calc();

int main()
{

#ifdef WITH_STRAND
  boost::asio::io_context service;
  boost::asio::io_context::strand my_strand(service);

  boost::asio::deadline_timer timer1(service, boost::posix_time::milliseconds(20000));
  timer1.async_wait(my_strand.wrap(handler));

  boost::asio::deadline_timer timer2(service, boost::posix_time::milliseconds(20000));
  timer2.async_wait(my_strand.wrap(handler));

  boost::asio::deadline_timer timer3(service, boost::posix_time::milliseconds(20000));
  timer3.async_wait(my_strand.wrap(handler));

  boost::asio::deadline_timer timer4(service, boost::posix_time::milliseconds(20000));
  timer4.async_wait(my_strand.wrap(handler));

  boost::asio::deadline_timer timer5(service, boost::posix_time::milliseconds(20000));
  timer5.async_wait(my_strand.wrap(handler));

  std::thread t1([&](){ service.run();});
  std::thread t2([&](){ service.run();});
  service.run();

  t1.join();
  t2.join();

#else

    boost::asio::io_context service;

    boost::asio::deadline_timer timer1(service, boost::posix_time::milliseconds(20000));
    timer1.async_wait(handler);

    boost::asio::deadline_timer timer2(service, boost::posix_time::milliseconds(20000));
    timer2.async_wait(handler);

    boost::asio::deadline_timer timer3(service, boost::posix_time::milliseconds(20000));
    timer3.async_wait(handler);

    boost::asio::deadline_timer timer4(service, boost::posix_time::milliseconds(20000));
    timer4.async_wait(handler);

    boost::asio::deadline_timer timer5(service, boost::posix_time::milliseconds(20000));
    timer5.async_wait(handler);

    std::thread t1([&](){ service.run();});
    std::thread t2([&](){ service.run();});
    service.run();

    t1.join();
    t2.join();

#endif

}
