//
// Created by embed on 14.11.16.
//

#ifndef WA_CLONE_KEYHIT_H
#define WA_CLONE_KEYHIT_H

#include <termios.h>
#include <unistd.h>

#include <functional>
#include <thread>
#include <iostream>

typedef std::function<void(char key)> KeyFunc;

class KeyHit {

  KeyFunc m_keyFunc;
  bool m_stop;
  struct termios m_term;
  std::thread th;

  void setTerminal() {
    if (tcgetattr(0, &m_term) < 0)
      perror("tcsetattr()");
    m_term.c_lflag &= ~ICANON;
    m_term.c_lflag &= ~ECHO;
    m_term.c_cc[VMIN] = 1;
    m_term.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &m_term) < 0)
      perror("tcsetattr ICANOW");
  }

  void resetTerminal() {
    m_term.c_lflag |= ICANON;
    m_term.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &m_term) < 0)
      perror("tcsetattr ~ICANON");
  }

  void readLine()
  {
    char buf(0);
    if (read(0, &buf, 1) < 0) {
      if (errno != EAGAIN)
        perror("read()");
      else
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    else
      if (m_keyFunc) m_keyFunc(buf);
  }
  void start() {
    th = std::thread([this]() {

      /* set terminal */
      setTerminal();

      while (!m_stop)
        readLine();

      /* reset terminal */
      resetTerminal();

      std::cout << "done with terminal\n";
    });
  }

public:
  KeyHit()
  : m_stop(false) {
    m_term = {0};
    start();
  }

  void setKeyReceiver(KeyFunc keyFunc) { m_keyFunc = keyFunc; }

  void stop() {
    // is called from another context
    m_stop = true;
    if (th.joinable())
      th.join();
  }


};


#endif //WA_CLONE_KEYHIT_H
