#ifndef TERMINAL_IO_H__
#define TERMINAL_IO_H__

#include <string>
#include <queue>

#include "thread.hh"

#define TERMINAL_BUF_SIZE 4096

void printListeners(const char *buf);

#define CONSOLE_PRINTF(fmt...) { \
		char buf[TERMINAL_BUF_SIZE];	 \
		snprintf(buf, sizeof(buf), fmt);		 \
		printListeners(buf); \
}

//		TerminalIOClass::printTerminal(buf);			\

class LineIOClass {
public:
  virtual void print(const std::string &str) = 0;
  virtual bool isDataAvailable()             = 0;
  virtual const string *getData()            = 0;
};

class TerminalIOClass : public ThreadedClass, public LineIOClass {
public:
  ThreadSafeQueue<std::string> inputQueue;
  ThreadSafeQueue<std::string> outputQueue;

	static LineIOClass *currentIO;
  static TerminalIOClass *terminalIO;
	static void printTerminal(const std::string &str) {
		if (currentIO != NULL) {
			currentIO->print(str);
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
