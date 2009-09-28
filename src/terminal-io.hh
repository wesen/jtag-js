#ifndef TERMINAL_IO_H__
#define TERMINAL_IO_H__

#include <string>
#include <queue>

#include "thread.hh"

class TerminalIOClass : public ThreadedClass {
public:
  ThreadSafeQueue<std::string> inputQueue;
  ThreadSafeQueue<std::string> outputQueue;

  static TerminalIOClass *terminalIO;

  TerminalIOClass(const char *name = NULL);
  virtual ~TerminalIOClass();
  
  void print(const std::string &str);

  static int readlinePoll();
  virtual void doWork();
};



#endif /* TERMINAL_IO_H__ */
