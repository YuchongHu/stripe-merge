#include "write_worker.hh"

#include <iostream>
#include <utility>
using namespace std::chrono;

WriteWorker::WriteWorker(uint t, MemoryPool* p)
    : ThreadPool<MigrationInfo>(t), mem_pool(p) {
  if (!p) {
    std::cerr << "Invalid MemoryPool pointer!" << std::endl;
    exit(-1);
  }
}

// override with WriteWorker's method to deal with tasks
void WriteWorker::run() {
  while (true) {
    // get a task from the task queue
    MigrationInfo task = std::move(get_task());
    if (!task.mem_ptr) {
      break;
    }

    // write file
    FILE* f = fopen(task.store_fn, "w");
    if (!f) {
      std::cerr << "fopen failed in WriteWorker: " << task.store_fn
                << std::endl;
      exit(-1);
    }
    ssize_t n, i = 0, remain = CHUNK_SIZE;
    while (i < CHUNK_SIZE && (n = fwrite(task.mem_ptr + i, 1, remain, f))) {
      i += n;
      remain -= n;
    }
    fclose(f);

    // release the block at last
    mem_pool->release_block(task.mem_ptr);
  }
}