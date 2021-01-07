#ifndef TCP_NODE_HH
#define TCP_NODE_HH

#include <string>
#include <vector>

#include "matching_generator.hh"
#include "ncscale_simulation.hh"
#include "tcp_client.hh"
#include "tcp_server.hh"

class TCPNode {
 private:
  TCPClient *client;
  TCPServer *server;
  uint16_t self_index;
  socket_address self_address;
  std::vector<socket_address> targets;
  uint16_t node_num;

  uint rs_k;
  uint rs_m;
  uint exp_size;
  uint N;
  uint scheme_type;

 public:
  explicit TCPNode(uint16_t i, uint _exp_size = 0, uint _scheme_type = 0);
  ~TCPNode();

  uint16_t parse_config(const std::string &fn);
  void start();
};

#endif
