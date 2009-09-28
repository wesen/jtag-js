#include "terminal-io.hh"

TerminalIOClass terminal;

#include "producer.hh"

ThreadSafeQueue<string> myQueue;
ConsumerClass consumer(&myQueue);
ProducerClass producer(&terminal.inputQueue, "pro1");

int main() {
  terminal.go();
  consumer.go();
  producer.go();

  while (1)
    ;
  
  return 0;
}
