#ifndef WRITE_WORKER_HH
#define WRITE_WORKER_HH

#include "ec_setting.hh"
#include "memory_pool.hh"
#include "migration_info.hh"
#include "thread_pool.hh"

class WriteWorker : public ThreadPool<MigrationInfo> {
 private:
  MemoryPool* mem_pool;

 public:
  WriteWorker(uint t, MemoryPool* p);
  ~WriteWorker() = default;

  void run() override;
};
#endif