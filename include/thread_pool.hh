#ifndef THREAD_POOL_HH
#define THREAD_POOL_HH

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>

/**        ThreadPool Template Class
 * The ThreadPool is consisted of some worker-threads, and a shared task queue.
 * These workers try to get a task constantly from the task queue once they are
 * started. If no task is available, they will fall in sleep and wait for being
 * notified. Note that the queue is guarded by a mutex to keep
 * multithread-safety, and the notification is realized via condition variable.
 *
 * By extending this ThreadPool Template Class, and overriding the run()
 * function, we can easily design various types of threadpool, but avoiding the
 * redundancy at the same time.
 */
template <typename Msg>
class ThreadPool {
 protected:
  uint thread_num;
  std::unique_ptr<std::thread[]> thrs;
  std::mutex queue_mtx;
  std::queue<Msg> task_queue;
  std::condition_variable queue_cv;
  bool seal_flag;

 public:
  explicit ThreadPool(uint t)
      : thread_num(t), thrs{new std::thread[t]}, seal_flag{false} {}
  ~ThreadPool() = default;

  // add the given task to the task queue
  void add_task(Msg msg) {
    std::unique_lock<std::mutex> lck(queue_mtx);
    task_queue.push(std::move(msg));
    lck.unlock();
    queue_cv.notify_one();
  }

  // try to get a task from the task queue in a blocking way
  Msg get_task() {
    std::unique_lock<std::mutex> lck(queue_mtx);
    // use condition variable to get notification
    while (!seal_flag && task_queue.empty()) {
      queue_cv.wait(lck, [&] { return seal_flag || !task_queue.empty(); });
    }
    if (!task_queue.empty()) {
      Msg m = task_queue.front();
      task_queue.pop();
      return m;
    } else {
      // empty task
      return Msg();
    }
  }

  // waiting to be overrided
  virtual void run() = 0;

  // start the worker-threads
  void start_threads() {
    for (uint i = 0; i < thread_num; ++i) {
      thrs[i] = std::thread([&] { this->run(); });
    }
  }

  // wait for the worker-threads to finish
  void wait_for_finish() {
    for (uint i = 0; i < thread_num; ++i) {
      thrs[i].join();
    }
  }

  // Set seal-flag and close the input of the queue, preventing new tasks from
  // adding. Then the workers will shutdown.
  void set_finished() {
    std::unique_lock<std::mutex> lock(queue_mtx);
    seal_flag = true;
    lock.unlock();
    queue_cv.notify_all();
  }
};
#endif