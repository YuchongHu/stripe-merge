#ifndef COMPUTE_WORKER_HH
#define COMPUTE_WORKER_HH

#include <isa-l.h>

#include <memory>

#include "ec_setting.hh"
#include "migration_info.hh"
#include "write_worker.hh"

// extend from ThreadPool; worker-threads are for ec computation
class ComputeWorker : public ThreadPool<MigrationInfo> {
 private:
  WriteWorker* ww;
  std::unique_ptr<uint8_t*[]> encode_gftbl;
  uint8_t rs_k;
  uint8_t rs_m;

  uint8_t get_gf_pow(uint8_t a, uint8_t k);

 public:
  explicit ComputeWorker(uint t, WriteWorker* p, uint8_t k, uint8_t m);
  ~ComputeWorker();

  void run() override;
  void init_ec_table();
};
#endif