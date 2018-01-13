#ifndef WA_CLONE_COMMANDLINE_H
#define WA_CLONE_COMMANDLINE_H

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <vector>
#include <functional>
#include <thread>
#include "KeyHit.h"

namespace posix = boost::asio::posix;

typedef std::function<void()> StopFunc;
typedef std::function<void(std::string)> SendFunc;

class CommandLine {

  boost::asio::io_service &m_service;
  posix::stream_descriptor m_output;
  std::vector<char> m_input_data;
  StopFunc m_stopFunc;
  SendFunc m_sendFunc;

  void handleCommandLineInput(char data)
  {
     if (data == '\n') {
       std::string cmdData(&m_input_data[0], m_input_data.size());
       if (cmdData == "quit") {
         m_output.cancel();
         m_output.close();
         m_stopFunc();
         return;
       }
       outputSimple('\n');
       m_sendFunc(cmdData);
       cmdData = "sending: <"+cmdData+">";
       m_input_data.clear();
       output(cmdData);
     }
    else {
       if (data==127) {
         m_input_data.pop_back();
         output("");
       }
       else {
         if (data > 31 && data < 127) {
           m_input_data.push_back(data);
           outputSimple(data);
         }
         else {
           std::stringstream str;
           str << "\nStrange sign <" << (int) data << "> received";
           output(str.str());
         }

       }

     }
  }

public:
  CommandLine(boost::asio::io_service& service)
          : m_service(service),
            m_output(service, ::dup(STDOUT_FILENO)) {
    m_input_data.reserve(255);
  }

  ~CommandLine() { std::cout << "CommandLine destructor\n"; }

  void setStop(StopFunc stopFunc) { m_stopFunc = stopFunc; }
  void setSend(SendFunc sendFunc) { m_sendFunc = sendFunc; }

  void keyInputExternal(char key) {
    m_service.post(std::bind(&CommandLine::handleCommandLineInput, this, key));
  }

  void outputSimple(char data) {
//    std::string output(&data,1);
    boost::asio::write(m_output, boost::asio::buffer(&data,1));
  }

  void output(const std::string& outputData) {

    std::shared_ptr<std::string> outputPtr;

    std::string seperator;
    if (outputData.empty())
      seperator = " \r";
    else
      seperator = "\n";

    std::string::size_type len = (m_input_data.size() > outputData.length() ? m_input_data.size() - outputData.length()
                                                                            : 0);
    std::string spaces(len, ' '); // overwrite all text written by the local user and reprint it on the next line
    outputPtr = std::make_shared<std::string>(
                    "\r" + outputData + spaces + seperator + std::string(&m_input_data[0], m_input_data.size()));

    boost::asio::async_write(m_output, boost::asio::buffer(*outputPtr),
                             [](const boost::system::error_code &error, const long unsigned int &) {
                               if (error)
                                 std::cout << error << std::endl;
                             });

  }

};


#ifdef WITH_EXEC
void output(std::shared_ptr<CommandLine> cmdl) {
  cmdl->output("\rhallo");
}
int main()
{
  boost::asio::io_service service;
  std::shared_ptr<CommandLine> cmdLine = std::make_shared<CommandLine>(service);

  boost::asio::deadline_timer timer(service, boost::posix_time::seconds(5));
  timer.async_wait(std::bind(&output, cmdLine));

  KeyHit keyHit(std::bind(&CommandLine::keyInputExternal, cmdLine, std::placeholders::_1));
  cmdLine->setStop(std::bind(&KeyHit::stop, &keyHit));
  cmdLine->output("starting");

  // keep service busy
  auto work = std::make_shared<boost::asio::io_service::work>(service);

  service.run();
  return 0;
}
#endif // WITH_EXEC

#endif //WA_CLONE_COMMANDLINE_H
