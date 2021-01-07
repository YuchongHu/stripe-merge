#include "compute_worker.hh"

#include <sys/time.h>

#include <iostream>
#include <utility>
using namespace std::chrono;

ComputeWorker::ComputeWorker(uint t, WriteWorker *p, uint8_t k, uint8_t m)
    : ThreadPool<MigrationInfo>(t),
      ww(p),
      encode_gftbl{new uint8_t *[m]},
      rs_k(k),
      rs_m(m) {
  init_ec_table();
}

uint8_t ComputeWorker::get_gf_pow(uint8_t a, uint8_t k) {
  if (a == 1) {
    return 1;
  }
  uint8_t result = a;
  for (uint8_t i = 1; i < k; ++i) {
    result = gf_mul(result, a);
  }
  return result;
}

void ComputeWorker::run() {
  // allocate memory for computing
  uint8_t **data = new uint8_t *[2]();
  data[1] = new uint8_t[CHUNK_SIZE]();
  while (true) {
    MigrationInfo task = std::move(get_task());
    if (!task.mem_ptr) {
      ww->set_finished();
      break;
    }

    // read target_file
    FILE *f = fopen(task.target_fn, "r");
    if (!f) {
      std::cerr << "fopen failed in compute_worker: " << task.target_fn
                << std::endl;
      exit(-1);
    }
    ssize_t n, i = 0, remain = CHUNK_SIZE;
    while (!feof(f) && (n = fread(data[1] + i, 1, remain, f)) > 0) {
      i += n;
      remain -= n;
    }
    fclose(f);

    // ISA-L erasure-coding computing
    data[0] = (uint8_t *)task.mem_ptr;
    ec_encode_data(CHUNK_SIZE, 2, 1, encode_gftbl[task.coefficient - 1], data,
                   data);
    ww->add_task(std::move(task));
  }

  delete[] data[1];
  delete[] data;
}

void ComputeWorker::init_ec_table() {
  uint8_t *encode_matrix = new uint8_t[6]{1, 0, 0, 1, 1, 1};
  uint8_t i;
  for (i = 0; i < rs_m; ++i) {
    encode_gftbl[i] = new uint8_t[64]();
    encode_matrix[5] = get_gf_pow(i + 1, rs_k);
    ec_init_tables(2, 1, encode_matrix, encode_gftbl[i]);
  }
  delete[] encode_matrix;
  // std::cout << "init_ec_table ok\n";
}

ComputeWorker::~ComputeWorker() {
  for (uint8_t i = 0; i < rs_m; ++i) {
    delete[] encode_gftbl[i];
  }
}