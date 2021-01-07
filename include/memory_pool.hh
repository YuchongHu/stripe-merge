#ifndef MEMORY_POOL_HH
#define MEMORY_POOL_HH

#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <unordered_map>

class MemoryPool {
 private:
  uint block_num;
  uint block_size;
  std::unique_ptr<char*[]> pool;
  std::mutex queue_mtx;
  std::queue<uint> free_queue;
  std::unordered_map<char*, uint> ptr_map;
  std::condition_variable queue_cv;

 public:
  MemoryPool() = default;
  ~MemoryPool();
  MemoryPool(uint n, uint s);

  char* get_block();
  void release_block(char* ptr);
};

#endif