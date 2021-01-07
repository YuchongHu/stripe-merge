#ifndef TCP_CLIENT_HH
#define TCP_CLIENT_HH

#include <sys/time.h>

#include <condition_variable>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>

#include "ec_setting.hh"
#include "migration_info.hh"
#include "sockpp/tcp_connector.h"
#include "thread_pool.hh"

typedef struct _s_a {
  std::string ip;
  in_port_t port;
  _s_a(const std::string& _ip, in_port_t _p) : ip(_ip), port(_p){};
  _s_a() = default;
} socket_address;

class TCPClient : public ThreadPool<MigrationInfo> {
 private:
  uint16_t node_index;
  in_port_t dest_port;
  std::string dest_host;
  sockpp::tcp_connector conn;
  std::unique_ptr<sockpp::tcp_connector[]> node_conns;
  std::unique_ptr<std::mutex[]> req_mtxs;
  uint16_t conn_num;

  void connect_one(sockpp::inet_address sock_addr, uint16_t index);
  void wait_start_flag(uint16_t index);

 public:
  TCPClient();
  TCPClient(uint16_t i, const std::string& _host, in_port_t _port);
  TCPClient(uint16_t i, std::vector<socket_address>& sa, uint thr_num = 1);
  ~TCPClient();

  void run() override;
  void start_client();
  void send_shutdown_signal();
  void demo_tasks();
};

#endif