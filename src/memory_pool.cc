#include "memory_pool.hh"

MemoryPool::MemoryPool(uint n, uint s) : block_num{n}, block_size{s} {
  pool.reset(new char*[n]);
  for (uint i = 0; i < n; ++i) {
    pool[i] = new char[s]();
    free_queue.push(i);
    ptr_map[pool[i]] = i;
  }
}

MemoryPool::~MemoryPool() {
  for (uint i = 0; i < block_num; ++i) {
    delete[] pool[i];
  }
}

char* MemoryPool::get_block() {
  std::unique_lock<std::mutex> lck(queue_mtx);
  while (free_queue.empty()) {
    queue_cv.wait(lck, [&] { return !free_queue.empty(); });
  }
  uint i = free_queue.front();
  free_queue.pop();
  return pool[i];
}

void MemoryPool::release_block(char* ptr) {
  auto it = ptr_map.find(ptr);
  if (it == ptr_map.end()) {
    std::cerr << "bad ptr in release_block\n";
    return;
  }
  std::unique_lock<std::mutex> lck(queue_mtx);
  free_queue.push(it->second);
  lck.unlock();
  queue_cv.notify_one();
}