#ifndef THREAD_POOL_HH
#define THREAD_POOL_HH

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>

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

  void add_task(Msg msg) {
    std::unique_lock<std::mutex> lck(queue_mtx);
    task_queue.push(std::move(msg));
    lck.unlock();
    queue_cv.notify_one();
  }

  Msg get_task() {
    std::unique_lock<std::mutex> lck(queue_mtx);
    while (!seal_flag && task_queue.empty()) {
      queue_cv.wait(lck, [&] { return seal_flag || !task_queue.empty(); });
    }
    if (!task_queue.empty()) {
      Msg m = task_queue.front();
      task_queue.pop();
      return m;
    } else {
      return Msg();
    }
  }

  virtual void run() = 0;

  void start_threads() {
    for (uint i = 0; i < thread_num; ++i) {
      thrs[i] = std::thread([&] { this->run(); });
    }
  }

  void wait_for_finish() {
    for (uint i = 0; i < thread_num; ++i) {
      thrs[i].join();
    }
  }

  void set_finished() {
    std::unique_lock<std::mutex> lock(queue_mtx);
    seal_flag = true;
    lock.unlock();
    queue_cv.notify_all();
  }
};
#endif