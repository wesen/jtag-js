#ifndef TERMINAL_IO_H__
#define TERMINAL_IO_H__

#include <string>
#include <queue>

#include "thread.hh"

#define TERMINAL_BUF_SIZE 4096

#define CONSOLE_PRINTF(fmt...) { \
		char buf[TERMINAL_BUF_SIZE];	 \
		snprintf(buf, sizeof(buf), fmt);		 \
		TerminalIOClass::printTerminal(buf); \
}

class TerminalIOClass : public ThreadedClass {
public:
  ThreadSafeQueue<std::string> inputQueue;
  ThreadSafeQueue<std::string> outputQueue;

  static TerminalIOClass *terminalIO;
	static void printTerminal(const std::string &str) {
		if (terminalIO != NULL) {
			terminalIO->print(str);
		}
	}

  TerminalIOClass(const char *name = NULL);
  virtual ~TerminalIOClass();
  
  void print(const std::string &str);
  bool isDataAvailable();
  const string *getData();

  static int readlinePoll();
  virtual void doWork();
};



#endif /* TERMINAL_IO_H__ */
