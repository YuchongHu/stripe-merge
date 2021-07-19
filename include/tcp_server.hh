#ifndef TCP_SERVER_HH
#define TCP_SERVER_HH

#include <sys/time.h>

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

#include "compute_worker.hh"
#include "ec_setting.hh"
#include "memory_pool.hh"
#include "migration_info.hh"
#include "sockpp/tcp_acceptor.h"
#include "thread_pool.hh"
#include "write_worker.hh"

// extend from ThreadPool; worker-threads are for receiving data
class TCPServer : public ThreadPool<MigrationInfo> {
 private:
  uint8_t rs_k;
  uint8_t rs_m;
  uint serving_num;
  in_port_t port;
  sockpp::tcp_acceptor acc;
  MemoryPool mem_pool;
  WriteWorker w_worker;
  ComputeWorker c_worker;
  std::mutex serving_mtx;
  std::condition_variable serving_cv;
  bool serving_flag;
  sockpp::tcp_socket master_sock;
  uint blocks_num;
  uint finished_num;
  std::mutex num_mtx;
  std::unique_ptr<std::thread[]> thr;
  void receive_handler(sockpp::tcp_socket sock);
  void run() override;

 public:
  TCPServer();
  TCPServer(in_port_t _port, uint n, uint8_t k, uint8_t m,
            uint workers_num = 1);
  ~TCPServer();

  void start_serving();
  void wait_for_connection();
};

#endif