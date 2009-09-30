#ifndef THREAD_H__
#define THREAD_H__

#include <assert.h>
#include <pthread.h>

#include <string>
#include <queue>

using namespace std;

#ifdef DEBUG
#define DEBUG_PRINTF(str, args...) \
  printf(str, ## args)
#else
#define DEBUG_PRINTF(stder, args...)
#endif

class ThreadSynchronization {
protected:
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  pthread_t owner;

public:
  ThreadSynchronization() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    owner = (pthread_t)-1;
  }

  virtual ~ThreadSynchronization() {
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
  }

  void lock() {
    DEBUG_PRINTF("lock %p from thread %d\n", this, pthread_self);
    pthread_mutex_lock(&mutex);
    owner = pthread_self();
  }

  void unlock() {
    DEBUG_PRINTF("unlock %p from thread %d (owner: %d)\n", this, pthread_self, owner);
    assert(owner == pthread_self());
    owner = (pthread_t)-1;
    pthread_mutex_unlock(&mutex);
  }

  void wait() {
    DEBUG_PRINTF("wait %p from thread %d (owner: %d)\n", this, pthread_self, owner);
    assert(owner == pthread_self());
    pthread_cond_wait(&cond, &mutex);
  }
  
  void signal() {
    DEBUG_PRINTF("signal %p from thread %d (owner: %d)\n", this, pthread_self, owner);
    assert(owner == pthread_self());
    pthread_cond_signal(&cond);
  }
};

template<class C> class ThreadSafeObject : public ThreadSynchronization {
protected:
  C *obj;

public:
  ThreadSafeObject(C *_obj) : ThreadSynchronization(), obj(_obj) {
  }

  virtual ~ThreadSafeObject() {
  }

  C *getLockedObject() {
    lock();
    return obj;
  }
};

class ThreadedClass {

public:
  ThreadedClass() : stopRequested(false), running(false) {
  }

  ~ThreadedClass() {
  }

  void go() {
    assert(running == false);
    running = true;
    pthread_create(&thread, NULL, &ThreadedClass::startThread, this);
  }

  void stop() {
    assert(running == true);
    running = false;
    stopRequested = true;
    //    pthread_join(&thread, 0);
  }

	bool isRunning() {
		return running;
	}

protected:
  pthread_t thread;
  
  volatile bool stopRequested;
  volatile bool running;

  static void *startThread(void *obj) {
    reinterpret_cast<ThreadedClass *>(obj)->doWork();
  }

  virtual void doWork() {
  }
};

template<class C> class ThreadSafeQueue : public ThreadSafeObject<queue<C> > {
protected:
  queue<C> myQueue;

public:
  ThreadSafeQueue() : ThreadSafeObject<queue<C> >(&myQueue) {
  }

  bool isDataAvailable() {
    queue<C> *q = ThreadSafeObject<queue<C> >::getLockedObject();
    bool result = !q->empty();
    ThreadSafeObject<queue<C> >::unlock();
    return result;
  }

  const C *getData() {
    queue<C> *q = ThreadSafeObject<queue<C> >::getLockedObject();
    C *result = new C(q->front());
    q->pop();
    ThreadSafeObject<queue<C> >::unlock();
    return result;
  }

  void putData(const C &data) {
    queue<C> *q = ThreadSafeObject<queue<C> >::getLockedObject();
    q->push(data);
    ThreadSafeObject<queue<C> >::unlock();
  }
};

#endif /* THREAD_H__ */
